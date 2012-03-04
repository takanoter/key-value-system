#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "kvs.h"
#include "type.h"
#include "layout.h"
#include "sync.h"
#include "index.h"
#include "disk.h"
#include "log.h"

static	char*	g_image=NULL;

static	char*	disk_file=NULL;
static	char*	disk_log=NULL;
static	char*	sync_log=NULL;
static	char*	index_log=NULL;

static	INIT_TYPE	init_type;
static	PTR_DISK	disk_vol_in_chunk=0;

static	int	_get_slot_cnt(int value_size);
static	void	_parse_init_set(const struct KVS_ENV* kvs);

int kv_init(const struct KVS_ENV* kvs)
{
	STATE state;
	assert(kvs);

	g_image=(char*)malloc(IMAGE_K_SIZE*1024);
	assert(g_image);

	_parse_init_set(kvs);	

	state=sync_init(disk_file, sync_log);	
	if (state!=SYNC_SUCCESS) {
		if (state==SYNC_NG_INIT_LOG) {
			printf ("kvs init error: open|create sync log file failed:%s.\n", sync_log);
			return KVS_INIT_NG_SYNC;
		} else if (state==SYNC_NG_FD) {
			printf ("kvs init error: open|create the big file failed:%s.\n", disk_file);
			return KVS_INIT_NG_BIG_FILE;
		}
	}

	if (init_type==INIT_TYPE_LOAD) {
		state=sync_read(DISK_IMAGE, g_image, IMAGE_K_SIZE*1024); 
		if (state!=SYNC_SUCCESS) {
			printf ("kvs init error: sync load image failed.\n");
			return KVS_INIT_NG_SYNC;
		}
	}

	state=idx_init(init_type, g_image+IMAGE_K_INDEX*1024, index_log);
	if (state!=IDX_SUCCESS) {
		if (state==IDX_NG_INIT_LOG) {
			printf ("kvs init error: open|create index log file failed:%s.\n", index_log);
			return KVS_INIT_NG_IDX_LOG;
		}
	}

	state=disk_init(init_type, g_image+IMAGE_K_DISK*1024, disk_log, disk_vol_in_chunk-DISK_IMAGE_CHUNK_SIZE);
	if (state!=DISK_SUCCESS) {
		if (state==DISK_NG_INIT_LOG) {
			printf ("kvs init error: open|create disk log file failed:%s.\n", disk_log);
			return KVS_INIT_NG_DISK_LOG;
		}
	}

	return KVS_INIT_SUCCESS;
}

void kv_image_flush( )
{
	STATE state;
	assert(g_image);
	state = sync_write(DISK_IMAGE, g_image, IMAGE_K_SIZE*1024);
	if (state != SYNC_SUCCESS) {
		printf ("kvs image flush error: sync failed.\n");
	}
}

int kv_exit()
{
	STATE state;
	char *p;

	disk_exit();
	idx_exit();

	if (init_type == INIT_TYPE_CREATE) {
		/*FIXME: disk_vol_in_chunk write_to DISK_IMAGE:IMAGE_K_SUPER */
		p = g_image;
	}

	state = sync_write(DISK_IMAGE, g_image, IMAGE_K_SIZE*1024);
	if (state != SYNC_SUCCESS) {
		printf ("kvs exit error: sync failed.\n");
		return KVS_EXIT_NG_SYNC;
	}

	free(g_image);
	sync_exit();
	return KVS_EXIT_SUCCESS;
}

int kv_get_index(const char* key, int key_size, struct KV_IDX* kv_idx)
{
	STATE state;
	assert(key);
	assert(kv_idx);

	state = idx_search(key, key_size, &(kv_idx->disk_location), &(kv_idx->value_size) );
	if (state != IDX_SUCCESS) {
		if (state == IDX_NG_NOT_EXIST) {
			printf ("kvs get error: key(%s) not exist.\n", key);
			return KVS_GET_NG_NOT_EXIST;
		}
	}

	return kv_idx->value_size;
}

int kv_read_index(const struct KV_IDX* kv_idx, char* buf, int buf_size)
{
	STATE state;
	assert(kv_idx);
	assert(buf);

	if (kv_idx->value_size > buf_size || kv_idx->value_size < 0) {
		printf ("kvs get inner fatal: Check by invalid value_size:%dB . or buffer(%dB) is small\n", kv_idx->value_size, buf_size);
		return KVS_GET_NG_INNER;
	}
		
	state = sync_read(DISK_VALUE+kv_idx->disk_location, buf, kv_idx->value_size);
	if (state != SYNC_SUCCESS) {
		printf ("kvs get error: location(%d) sync failed.\n", kv_idx->disk_location);
		return KVS_GET_NG_SYNC;
	}

	return kv_idx->value_size; 
}

int kv_get(const char* key, int key_size, char* buf, int buf_size)
{
	int value_size;
	PTR_DISK disk_location;	
	STATE state;
	assert(key);
	assert(buf);

	state = idx_search(key, key_size, &disk_location, &value_size);
	if (state != IDX_SUCCESS) {
		if (state == IDX_NG_NOT_EXIST) {
			printf ("kvs get error: key(%s) not exist.\n", key);
			return KVS_GET_NG_NOT_EXIST;
		}
	}
	
	if (value_size > buf_size || value_size < 0) {
		printf ("kvs get inner fatal: Check by invalid value_size:%dB of the key (%s). or buffer(%dB) is small\n", value_size, key, buf_size);
		return KVS_GET_NG_INNER;
	}

	state = sync_read(DISK_VALUE+disk_location, buf, value_size);
	if (state != SYNC_SUCCESS) {
		printf ("kvs get error: key(%s) sync failed.\n", key);
		return KVS_GET_NG_SYNC;
	}

	return value_size;
}

int kv_put(const char* key, int key_size, const char* value, int value_size)
{
	int slot_cnt;
	PTR_DISK disk_location;
	STATE state;
	assert(key);
	assert(value);

	if (value_size >= 5*1024*1024 || value_size < 0) {
		printf ("kvs put error: key(%s) with BIG value size:%dB (>%dK).\n", key, value_size, 5*1024);
		return KVS_PUT_NG_VALUE_SIZE;
	}
	
	slot_cnt = _get_slot_cnt(value_size);

	disk_location = disk_alloc(slot_cnt);
	if (disk_location == DISK_NG_SLOT_CNT) {
		printf ("kvs put error: key:%s value size:%dK but failed.\n", key, value_size/1024);
		return KVS_PUT_NG_VALUE_SIZE;
	} else if (disk_location == DISK_NG_LIMIT_OUT) {
		printf ("kvs put error: disk is full, no more free space.\n");
		return KVS_PUT_NG_DISK_SPACE_OUT;
	}

	state = idx_insert(key, key_size, disk_location, value_size);
	if (state != IDX_SUCCESS) {
		if (state == IDX_NG_EXIST) {
			printf ("kvs put error: key(%s) is exist.\n", key);
			return KVS_PUT_NG_EXIST;
		}
	}

	state = sync_write(DISK_VALUE+disk_location, value, value_size);
	if (state != SYNC_SUCCESS) {
		printf ("kvs put error: key(%s) sync failed.\n", key);
		return KVS_PUT_NG_SYNC;
	}

	return KVS_PUT_SUCCESS;
}

int kv_delete(const char* key, int key_size)
{
	PTR_DISK disk_location;
	STATE state;
	assert(key);

	state = idx_delete(key, key_size, &disk_location);
	if (state != IDX_SUCCESS) {
		printf ("kvs delete error: key(%s) is not exist.\n", key);
        	return KVS_DELETE_NG_NOT_EXIST;
	}
    
	state = disk_free(disk_location);	
	if (state != DISK_SUCCESS) {
		printf ("kvs delete inner fatal: key(%s) can't find the proper VALUE_CHUNK.\n", key);
		return KVS_DELETE_NG_DISK_FREE;
	}
	
	return KVS_DELETE_SUCCESS;
}

static int _get_slot_cnt(int value_size)
{
	return value_size/CHUNK_SIZE+1;
}


static void _parse_init_set(const struct KVS_ENV* kvs)
{
	init_type = kvs->init_type;
	disk_file = kvs->disk_file;
	disk_log = kvs->disk_log;
	sync_log = kvs->sync_log;
	index_log = kvs->index_log;
	disk_vol_in_chunk = (kvs->size_in_g)*64*1024; 
}
