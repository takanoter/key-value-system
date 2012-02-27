#define	_FILE_OFFSET_BITS	64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "data.h"

typedef struct DATA_HEAD_LAYOUT {
    int magic_num;
	int version;

    /* describe a snapshot of data  */
    unsigned long long kv_count;
    int append_offset;
    unsigned long long timestamp;
	
    /* KV_ITEM + key + value */
} DATA_HEAD_LAYOUT;

typedef struct KV_ITEM_HEAD {
    unsigned long long timestamp;
    unsigned long long key_len;
    unsigned long long value_len;
}

static DATA_HEAD_LAYOUT _datahead_create()
{
    DATA_HEAD_LAYOUT data_head;
    data_head.magic_num = 1806;
    data_head.version = 0;
    data_head.kv_count = 0;
    data_head.append_offset = sizeof(DATA_HEAD_LAYOUT);
    data_head.timestamp = 0;
    return data_head; 
}

static DATA_HEAD_LAYOUT _datahead_fetch(DATA* data)
{
    DATA_HEAD_LAYOUT data_head;
    data_head.magic_num = 1806;
    data_head.version = 0;
    data_head.kv_count = data->kv_count;
    data_head.append_offset = data->append_offset;
    data_head.timestamp = data->timestamp;
    return data_head;
}

static int _datahead_read(int fd, DATA_HEAD_LAYOUT* datahead)
{
    int ret = 0;

	ret = lseek(fd, 0, SEEK_SET);
    if (ret < 0) {
        return -1;
    
    
    ret = read(data->fd, datahead, sizeof(*datahead));
    if (ret < 0) {
        return -1;
    }
    /*TODO: some check here, such as magic_num, version*/
    
    return err_success;
}

DATA* data_create(const char* filename)
{
    DATA* data = NULL;
    int fd = -1;

	fd = open(filename, O_RDWR | O_LARGEFILE | O_CREAT , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); 
	if (fd <= 0) 
	{
        printf ("open [%s] failed.\n");
        goto OUT;
	}

    DATA_HEAD_LAYOUT datahead = _datahead_create();

    data = (DATA*) malloc(sizeof(DATA)); 
    if (NULL == data) {
        goto OUT;
    }
    data->fd = fd;
    data->kv_count = datahead.kv_count;
    data->append_offset = datahead.append_offset;
    data->timestamp = datahead.timestamp;
    
    ret = data_head_sync(data);
    if (err_success != ret) {
        goto OUT;
    }

    return data;

OUT:
    if (NULL != data) {
        free(data);
        data = NULL;
    }

    return NULL;
}

DATA* data_load(const char* filename)
{
    DATA* data = NULL;
    int fd = -1;

	fd = open(filename, O_RDWR | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); 
	if (fd <= 0) 
	{
        printf ("load [%s] failed.\n");
        goto OUT;
	}

    DATA_HEAD_LAYOUT datahead = _datahead_read(fd);

    data = (DATA*) malloc(sizeof(DATA)); 
    if (NULL == data) {
        goto OUT;
    }
    data->fd = fd;
    data->kv_count = datahead.kv_count;
    data->append_offset = datahead.append_offset;
    data->timestamp = datahead.timestamp;
    
    return data;

OUT:
    if (NULL != data) {
        free(data);
        data = NULL;
    }

    return NULL;
}

int data_head_sync(DATA* data)
{
    int ret = err_success;

    DATA_HEAD_LAYOUT datahead = _datahead_fetch(data);

	ret = lseek(data->fd, 0, SEEK_SET);
    if (ret < 0) {
        return -1;
    }
    
    write(data->fd, &datahead, sizeof(datahead));
    return 0;
}

int data_insert(DATA* data, unsigned long long value_offset, KV_PAIR* kv_pair)
{
    int ret = 0;
    unsigned long long kv_item_size = 0;
    KV_ITEM_HEAD kv_item_head;

    kv_item_head.timestamp = kv_pair->timestamp;
    kv_item_head.key_len = kv_pair->key_len;
    kv_item_head.value_len = kv_pair->value_len;
    
    kv_item_size = sizeof(KV_PAIR_HEAD) + kv_pair->key_len + kv_pair->value_len;
    
	ret = lseek(data->fd, value_offset, SEEK_SET);
    if (ret < 0) {
        return -1;
    }
    ret = write(data->fd, &kv_item_head, sizeof(kv_item_head));
    ret = write(data->fd, kv_pair->key, kv_pair->key_ken);
    ret = write(data->fd, kv_pair->value, kv_pair->value_len);
    data->append_offset += kv_item_size;
    data->kv_count = data->kv_count + 1;
    data->timestamp = kv_pair->timestamp;
    
    return 0;
}

int data_get(DATA* data, unsigned long long value_offset, KV_PAIR* kv_pair) 
{
    int ret = 0;
    KV_ITEM_HEAD kv_item_head;
    char* buffer = NULL;
    char* p = NULL;
    
    ret = lseek(data->fd, value_offset, SEEK_SET);
    if (ret < 0) {
        return -1;
    }

    ret = read(data->fd, &kv_item_head, sizeof(kv_item_head));
    if (ret < 0) {
        return -1;
    }
    
    buffer = (char*) malloc(kv_item_head.key_len + kv_item_head.value_len + 16); 
    if (NULL == buffer) {
        ret = err_malloc_failed;
        goto OUT;
    }
    ret = read(data->fd, buffer, kv_item_head.key_len);
    p = buffer + ret;
    ret = read(data->fd, p, kv_item_head.value_len);

    /* fill in kv_pair */
    kv_pair->key = buffer;
    kv_pair->key_len = kv_item_head.key_len;
    kv_pair->value = p;
    kv_pair->value_len = kv_item_head.value_len;

    return 0;

OUT:
    if (NULL != buffer) {
        free(buffer);
        buffer = NULL;
    }

    return ret;
}

int data_delete(DATA* data, unsigned long long timestamp) 
{
    data->kv_count = data->kv_count - 1;
    data->timestamp = timestamp;
    return err_success;
}

