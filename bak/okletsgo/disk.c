/*
 * A better Algorithm to be implemented: Bigger-Fill Algorithm.
 * Every time get space in the BIGGEST continious chunks
 * Reason: if disk has many fragments, and disk-fragment-state over some horizon, just change another disk
 *         Performance >> Use rate
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "type.h"
#include "layout.h"
#include "disk.h"
#include "log.h"

#define	HASH_DELETE_FAIL	-4
#define	NOT_HAVE_ENOUGH_SPACE	-3
#define	PTR_UNAVAILABLE		-1

/*FIXME: naming */
#define	HASH_MOD		131071
#define	HASH_LENGTH		131072
#define	NODE_ID_POOL_MAX_CNT	100024		/* FIXME: no reason,100024 */

#define MAX_CHUNK_CNT		320
#define	FREE_LIST_LENGTH	322


typedef struct NODE
{
	int ptr_disk;
	int flist_pre, flist_suc;
	int node_pre, node_suc;
	int size;
	int used;
}NODE;

static	NODE*	nodes=NULL;
static	int*	nodes_horizon=0;
static	int*	free_list=NULL;
static	int*	hash=NULL;
static	int*	node_id_pool=NULL;
static	int*	node_id_pool_horizon = 0;

static	FILE*	log_file=NULL;

static	void	hash_insert(int, int);
static	int	hash_delete(int);
static	int	alloc_node_pool();
static	void	free_node_pool(int);
static	int	get_free_list(int);
static	void	put_free_list(int);
static	void	delete_free_list_by_id(int);



/*
 disk_size_in_chunk is DISK_MOD manage chunk size, not the whole hardisk chunk size. 
 */
STATE disk_init(INIT_TYPE init_type, char* image, const char* disk_log_file, const int disk_size_in_chunk)
{
	int i;

	if (disk_log_file) {
		log_file=fopen(disk_log_file, "w"); /*XXX: append write or not?? */
		if (log_file==NULL) {
			printf ("[%s:%d] %s: LOG_FILE:%s open failed. File may be exist, or try using SUDO. \n", __FILE__, __LINE__, strerror(errno), disk_log_file);
			return DISK_NG_INIT_LOG;
		}
	}

	/* init the DiskManagerImage Layout
	 */
	nodes=(NODE*) (image+IMAGE_D_NODES);
	nodes_horizon=(int*)(image+IMAGE_D_NODES_HORIZON);
	free_list=(int*)(image+IMAGE_D_FREE_LIST);
	hash=(int*)(image+IMAGE_D_HASH);
	node_id_pool=(int*)(image+IMAGE_D_NODE_ID_POOL);
	node_id_pool_horizon=(int*)(image+IMAGE_D_NODE_ID_POOL_HORIZON);

	if (init_type==INIT_TYPE_CREATE) {
		for (i = 0; i < FREE_LIST_LENGTH; i++) {
			free_list[i] = PTR_UNAVAILABLE;
		}
		for (i = 0; i < HASH_LENGTH; i++) {
			hash[i] = PTR_UNAVAILABLE;
		}
		*nodes_horizon=1; /* because nodes[0] is used */
		*node_id_pool_horizon=0;
		
		nodes[0].used=0;
		nodes[0].size=disk_size_in_chunk;
		nodes[0].ptr_disk=0;
		nodes[0].node_pre=nodes[0].node_suc=PTR_UNAVAILABLE;

		put_free_list(0);
	}

	if (log_file) {
		fprintf (log_file, "[INIT]: sync init success.\n");
	} else {
		printf ("disk init success.\n");
	}

	return DISK_SUCCESS;
}

/*FIXME: return value? */
void disk_exit()
{
	if (log_file) {
		fprintf (log_file, "EXIT.\n");
		fclose(log_file);
	} else {
		printf ("disk exit.\n");
	}
	log_file=NULL;
	return;
}


/* 
 *FIXME: return value & return result. 
 */
int disk_alloc(int slot_cnt)
{
	if (slot_cnt > MAX_CHUNK_CNT || slot_cnt <= 0) {
		log_err(__FILE__, __LINE__, log_file, "Invalid slot_cnt:%d", slot_cnt);
		return DISK_NG_SLOT_CNT;
	}

	if (free_list[slot_cnt] != PTR_UNAVAILABLE) {
		/*  just equal to slot_cnt */
		int node_id = get_free_list(slot_cnt);
		nodes[node_id].used = 1;// size, ptr_disk, node_pre, node_suc are not change, flist_pre and flist_suc have changed at func:get_free_list
		hash_insert(nodes[node_id].ptr_disk, node_id);
		return nodes[node_id].ptr_disk;
	} else {
		/* bigger than slot_cnt */
		int free_node_id = get_free_list(slot_cnt);
		if (free_node_id == NOT_HAVE_ENOUGH_SPACE) {
			log_err(__FILE__, __LINE__, log_file, "Not have enough space.");
			return DISK_NG_LIMIT_OUT;
		}

		int node_id = alloc_node_pool(); 

		nodes[node_id].used = 1;
		nodes[node_id].size = slot_cnt;
		nodes[node_id].ptr_disk = nodes[free_node_id].ptr_disk;
		nodes[node_id].node_pre = nodes[free_node_id].node_pre;
		nodes[node_id].node_suc = free_node_id;

		nodes[free_node_id].used = 0;
		nodes[free_node_id].size -= slot_cnt;
		nodes[free_node_id].ptr_disk = nodes[free_node_id].ptr_disk + slot_cnt;
		if (nodes[free_node_id].node_pre !=PTR_UNAVAILABLE) {
			nodes[nodes[free_node_id].node_pre].node_suc = node_id;
		}
		nodes[free_node_id].node_pre = node_id;
		//nodes[free_node_id].node_suc not change

		put_free_list(free_node_id);
		hash_insert(nodes[node_id].ptr_disk, node_id);

		return nodes[node_id].ptr_disk;
	}

	printf ("error. should not be here:%s, %d\n",__FILE__, __LINE__);
	return 0;
}


/*FIXME: return value? */
STATE disk_free(int location)
{
	int node_id;

	node_id = hash_delete(location);

	if (node_id == HASH_DELETE_FAIL) {
		log_err(__FILE__, __LINE__, log_file, "Invalid free location:%d", location);
		return DISK_NG_FREE;
	}

	int node_pre = nodes[node_id].node_pre;
	int node_suc = nodes[node_id].node_suc;

	if (node_pre != PTR_UNAVAILABLE && !nodes[node_pre].used) {
		/* free buddy to left */
		while (!nodes[node_pre].used) {
			nodes[node_id].used = 0;
			nodes[node_id].size += nodes[node_pre].size;
			nodes[node_id].ptr_disk = nodes[node_pre].ptr_disk;
			nodes[node_id].node_pre = nodes[node_pre].node_pre;
			//nodes[node_id].node_suc not change
			if (nodes[node_pre].node_pre != PTR_UNAVAILABLE) {
				nodes[nodes[node_pre].node_pre].node_suc = node_id;
			}
			delete_free_list_by_id(node_pre);
			free_node_pool(node_pre);
			node_pre = nodes[node_id].node_pre;
		}
	} else if (node_suc != PTR_UNAVAILABLE && !nodes[node_suc].used) {
		/* free buddy to right */
		while (!nodes[node_suc].used) {
			nodes[node_id].used = 0;
			nodes[node_id].size += nodes[node_suc].size;
			//nodes[node_id].ptr_disk not change
			//nodes[node_id].node_pre not change
			nodes[node_id].node_suc = nodes[node_suc].node_suc;
			if (nodes[node_suc].node_suc != PTR_UNAVAILABLE) {
				nodes[nodes[node_suc].node_suc].node_pre = node_id;
			}
			delete_free_list_by_id(node_suc);
			free_node_pool(node_suc);
			node_suc = nodes[node_id].node_suc;
		}
	} else {
		/* free itself */
		nodes[node_id].used = 0;
	}

	put_free_list(node_id);
	return DISK_SUCCESS;
}


static void hash_insert(int disk_location, int node_id)
{
	int hash_id, cur_id;
	int* pre_ptr;

	hash_id = disk_location % HASH_MOD;
	nodes[node_id].used = 1;
	nodes[node_id].ptr_disk = disk_location;
	nodes[node_id].flist_pre = PTR_UNAVAILABLE;
	nodes[node_id].flist_suc = PTR_UNAVAILABLE;

	if (hash[hash_id] == PTR_UNAVAILABLE) {
		hash[hash_id] = node_id;
		return;
	}

	cur_id = hash[hash_id];
	do {
		if (disk_location > nodes[cur_id].ptr_disk) {
			pre_ptr = &nodes[cur_id].flist_suc;
			cur_id = nodes[cur_id].flist_suc;
		} else if (disk_location < nodes[cur_id].ptr_disk) {
			pre_ptr = &nodes[cur_id].flist_pre;
			cur_id = nodes[cur_id].flist_pre;
		}
	} while (cur_id != PTR_UNAVAILABLE);
	*pre_ptr = node_id;
}


static int hash_delete(int location)
{
	int hash_id, result_id, cur_id;
	int* pre_ptr;
	int right_id, right_left_id;

	hash_id = location % HASH_MOD;
	pre_ptr = &hash[hash_id];
	cur_id = hash[hash_id];
	if (cur_id == PTR_UNAVAILABLE) {
		return HASH_DELETE_FAIL;
	}

	while (cur_id != PTR_UNAVAILABLE) {
		if (location > nodes[cur_id].ptr_disk) {
			pre_ptr = &nodes[cur_id].flist_suc;
			cur_id = nodes[cur_id].flist_suc;
			if (cur_id == PTR_UNAVAILABLE) {
				return HASH_DELETE_FAIL;//not have location in hash
			}
		} else if (location < nodes[cur_id].ptr_disk) {
			pre_ptr = &nodes[cur_id].flist_pre;
			cur_id = nodes[cur_id].flist_pre;		
			if (cur_id == PTR_UNAVAILABLE) {
				return HASH_DELETE_FAIL;//not have location in hash
			}
		} else {
			break;
		}
	}

	assert(nodes[cur_id].ptr_disk == location);
	result_id = cur_id;

	if ((nodes[cur_id].flist_pre == PTR_UNAVAILABLE) && (nodes[cur_id].flist_suc == PTR_UNAVAILABLE)) {
		*pre_ptr = PTR_UNAVAILABLE;
		return result_id;
	}

	if ((nodes[cur_id].flist_pre != PTR_UNAVAILABLE) && (nodes[cur_id].flist_suc == PTR_UNAVAILABLE)) {
		*pre_ptr = nodes[cur_id].flist_pre;
		return result_id;
	}

	if ((nodes[cur_id].flist_pre ==PTR_UNAVAILABLE) && (nodes[cur_id].flist_suc != PTR_UNAVAILABLE)) {
		*pre_ptr = nodes[cur_id].flist_suc;
		return result_id;
	}

	right_id = nodes[cur_id].flist_suc;
	right_left_id = right_id;
	while (nodes[right_left_id].flist_pre != PTR_UNAVAILABLE) {
		right_left_id = nodes[right_left_id].flist_pre;
	}

	nodes[right_left_id].flist_pre = nodes[cur_id].flist_pre;
	*pre_ptr = right_id;
	return result_id;
}

static int alloc_node_pool()
{
	int ret;
	if ((*node_id_pool_horizon) == 0)  {
		ret = *nodes_horizon;
		(*nodes_horizon)++;
	} else {
		ret = node_id_pool[--(*node_id_pool_horizon)];
	}
	return ret;
}

static void free_node_pool(int node_id)
{
	node_id_pool[*node_id_pool_horizon] = node_id;
	if ((*node_id_pool_horizon)==NODE_ID_POOL_MAX_CNT) {
		printf ("out of limit @ free_node_pool(), disk.c\n");
	}
	(*node_id_pool_horizon) = ((*node_id_pool_horizon) + 1) % NODE_ID_POOL_MAX_CNT;
}

static int get_free_list(int slot_cnt)
{
	int i = slot_cnt;

/* FIXME: Can be faster?
 */ 
	while (free_list[i] == PTR_UNAVAILABLE && i < FREE_LIST_LENGTH)
		i++;

	if (i >= FREE_LIST_LENGTH) {
		printf("not have enough space at get_free_list\n");
		return NOT_HAVE_ENOUGH_SPACE;
	} 

	int node_id = free_list[i];
	free_list[i] = nodes[node_id].flist_suc;
	if (nodes[node_id].flist_suc != PTR_UNAVAILABLE) {
		nodes[nodes[node_id].flist_suc].flist_pre = PTR_UNAVAILABLE;
	}

	return node_id;
}

static void put_free_list(int node_id)
{
	int free_list_id = nodes[node_id].size > MAX_CHUNK_CNT ? (MAX_CHUNK_CNT+1) : nodes[node_id].size;

	nodes[node_id].flist_pre = PTR_UNAVAILABLE;
	nodes[node_id].flist_suc = free_list[free_list_id];
	if (nodes[node_id].flist_suc != PTR_UNAVAILABLE) {
		nodes[nodes[node_id].flist_suc].flist_pre = node_id;
	}
	free_list[free_list_id] = node_id;
}

static void delete_free_list_by_id(int node_id)
{
	int free_list_id = nodes[node_id].size > MAX_CHUNK_CNT ? (MAX_CHUNK_CNT+1) : nodes[node_id].size;
	int flist_pre = nodes[node_id].flist_pre;
	int flist_suc = nodes[node_id].flist_suc;

	if (flist_pre == PTR_UNAVAILABLE) {
		free_list[free_list_id] = flist_suc;
		if (flist_suc != PTR_UNAVAILABLE) {
			nodes[flist_suc].flist_pre = PTR_UNAVAILABLE;
		}
	} else {
		nodes[flist_pre].flist_suc = flist_suc;
		if (flist_suc != PTR_UNAVAILABLE) {
			nodes[flist_suc].flist_pre = flist_pre;
		}
	}
}

/* DO NOT DELETE 
 * just check 1024*char 
 */
/*
void disk_show_hash(const char* bitmap_file);
void disk_show_flist(const char* bitmap_file)
{
	FILE* fd=NULL;
	char *bitmap;

	if (bitmap_file==NULL) {
		printf ("bitmap_file is null.\n");
		return;
	}

	if (nodes==NULL || hash==NULL || free_list==NULL) {
		printf ("Not init.\n");
		return;
	}

	fd=fopen(bitmap_file, "w");
	bitmap=mp_alloc(1024);
	for (i=0; i<1024; i++) 
		bitmap[i]=0;
}
*/
