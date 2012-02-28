#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#include "index.h"
#include "kvs_utils.h"

#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define INDEX_NODE_NOT_EXIST 123456789
#define THIS_IS_HASH_HEAD_NUM 1024
#define THIS_IS_FREE_NODE_POOL_SIZE (1024*1024)

static INDEX_HEAD_LAYOUT _indexhead_create(unsigned long long key_num_quotas, unsigned long long timestamp);
static INDEX_HEAD_LAYOUT _indexhead_read(int fd);
static INDEX_HEAD_LAYOUT _indexhead_fetch(INDEX* index);
static void _put_free_idx_node(INDEX* idx, unsigned long long node_id);
static unsigned long long _get_free_idx_node(INDEX *idx);
static int _is_hash_same(unsigned long long a, unsigned long long b);

int idx_sync(INDEX* index)
{
    int ret = err_success;

    INDEX_HEAD_LAYOUT indexhead = _indexhead_fetch(index);

	ret = lseek(index->fd, 0, SEEK_SET);
    if (ret < 0) {
        return -1;
    }
    
    write(index->fd, &indexhead, sizeof(indexhead));
    write(index->fd, index->node_pool, 
            index->node_pool_size * sizeof(*(index->node_pool)) );
    write(index->fd, index->hash_head, 
            index->hashhead_size * sizeof(*(index->hash_head)) );
    write(index->fd, index->free_node_pool, 
            index->free_node_pool_size * sizeof(*(index->free_node_pool)) );

    index->is_change = 0;
    return 0;
}

INDEX* idx_create(const char *filename, unsigned long long key_num_quotas)
{
    INDEX* index = NULL;
    int fd = -1;
    int i = 0;
    int ret = 0;

	fd = open(filename, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); 
	if (fd <= 0) 
	{
        printf ("open [%s] failed.\n", filename);
        goto OUT;
	}

    unsigned long long timestamp = _kv_consequence_id(); 
    INDEX_HEAD_LAYOUT indexhead = _indexhead_create(key_num_quotas, timestamp);

    index = (INDEX*) malloc(sizeof(INDEX)); 
    if (NULL == index) {
        goto OUT;
    }

    index->fd = fd;
    index->is_change = 0;
    index->cur_kv_count = indexhead.cur_kv_count;
    index->hashhead_size = indexhead.hash_head;
    index->hash_head = (unsigned long long*) malloc(sizeof(*(index->hash_head)) * indexhead.hash_head);
    for (i = 0; i< index->hashhead_size; i++) {
        index->hash_head[i] = INDEX_NODE_NOT_EXIST;
    }
    index->node_pool_size = indexhead.node_pool_size;
    index->node_pool = (IDX_NODE*) malloc(sizeof(*(index->node_pool)) * indexhead.node_pool_size);
    index->free_node_pool_size = indexhead.free_node_pool_size;
    index->free_node_count = indexhead.free_node_count;
    index->free_node_pool = (unsigned long long*) malloc(sizeof(*(index->free_node_pool) * indexhead.free_node_pool_size));
    index->timestamp = indexhead.timestamp;
    index->node_pool_horizon = indexhead.node_pool_horizon;
    
    ret = idx_sync(index);
    if (err_success != ret) {
        goto OUT;
    }

    return index;

OUT:
    close(index->fd);
    if (NULL != index->hash_head) {
        free(index->hash_head);
        index->hash_head = NULL;
    }
    if (NULL != index->free_node_pool) {
        free(index->free_node_pool);
        index->free_node_pool = NULL;
    }
    if (NULL != index->node_pool) {
        free(index->node_pool);
        index->node_pool = NULL;
    }

    if (NULL != index) {
        free(index);
        index= NULL;
    }

    return NULL;
}

INDEX* idx_load(const char *filename)
{
    INDEX* index = NULL;
    int fd = -1;

	fd = open(filename, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); 
	if (fd <= 0) {
        printf ("load [%s] failed.\n", filename);
        goto OUT;
	}

    INDEX_HEAD_LAYOUT indexhead = _indexhead_read(fd);

    index = (INDEX*) malloc(sizeof(INDEX)); 
    if (NULL == index) {
        goto OUT;
    }

    index->fd = fd;
    index->is_change = 0;
    index->cur_kv_count = indexhead.cur_kv_count;
    index->hashhead_size = indexhead.hash_head;
    index->hash_head = (unsigned long long*) malloc(sizeof(*(index->hash_head)) * indexhead.hash_head);
    index->node_pool_size = indexhead.node_pool_size;
    index->node_pool = (IDX_NODE*) malloc(sizeof(*(index->node_pool)) * indexhead.node_pool_size);
    index->free_node_pool_size = indexhead.free_node_pool_size;
    index->free_node_count = indexhead.free_node_count;
    index->free_node_pool = (unsigned long long*) malloc(sizeof(*(index->free_node_pool) * indexhead.free_node_pool_size));
    index->timestamp = indexhead.timestamp;
    index->node_pool_horizon = indexhead.node_pool_horizon;

    lseek(index->fd, sizeof(INDEX_HEAD_LAYOUT), SEEK_SET);
    read(index->fd, index->node_pool,
            index->node_pool_size * sizeof(*(index->node_pool)) );
    read(index->fd, index->hash_head,
            index->hashhead_size * sizeof(*(index->hash_head)) );
    read(index->fd, index->free_node_pool,
            index->free_node_pool_size * sizeof(*(index->free_node_pool)) );
    
    return index;

OUT:
    close(index->fd);
    if (NULL != index->hash_head) {
        free(index->hash_head);
        index->hash_head = NULL;
    }
    if (NULL != index->free_node_pool) {
        free(index->free_node_pool);
        index->free_node_pool = NULL;
    }
    if (NULL != index->node_pool) {
        free(index->node_pool);
        index->node_pool = NULL;
    }
    
    if (NULL != index) {
        free(index);
        index = NULL;
    }

    return NULL;
}

int idx_exit(INDEX *index)
{
    if (0 != index->is_change) {
        idx_sync(index);
    }
    close(index->fd);
    if (NULL != index->hash_head) {
        free(index->hash_head);
        index->hash_head = NULL;
    }
    if (NULL != index->free_node_pool) {
        free(index->free_node_pool);
        index->free_node_pool = NULL;
    }
    if (NULL != index->node_pool) {
        free(index->node_pool);
        index->node_pool = NULL;
    }
    
    if (NULL != index) {
        free(index);
        index = NULL;
    }

	return 0;
}

int idx_search(INDEX* idx, IDX_NODE* node)
{
    int cmp = 0;
    unsigned long long node_id = 0;
	unsigned long long ht_table_id = 0; 
    unsigned long long new_node_id = 0;
	unsigned long long*	pre_node_ptr = NULL;

    ht_table_id = _get_hashhead_id(node->key_sign, idx->hashhead_size);
	node_id = idx->hash_head[ht_table_id];

	while(node_id != INDEX_NODE_NOT_EXIST)
	{	
		cmp = _is_hash_same(idx->node_pool[node_id].key_sign, node->key_sign);
		if (cmp == 0) {
            node->value_offset = idx->node_pool[node_id].value_offset;
			return 0;
		} else if (cmp < 0) {
			node_id = idx->node_pool[node_id].left_id;
		} else {
			node_id = idx->node_pool[node_id].right_id;
		}
	}
	
    printf ("key not found.\n");
	return -1;	
}

int idx_insert(INDEX* idx, const IDX_NODE* node, unsigned long long timestamp)
{
    int cmp = 0;
    unsigned long long node_id = 0;
	unsigned long long ht_table_id = 0; 
    unsigned long long new_node_id = 0;
	unsigned long long*	pre_node_ptr = NULL;

    ht_table_id = _get_hashhead_id(node->key_sign, idx->hashhead_size);
	pre_node_ptr = &idx->hash_head[ht_table_id];
	node_id = idx->hash_head[ht_table_id];

	while (node_id != INDEX_NODE_NOT_EXIST)
	{
		cmp = _is_hash_same(idx->node_pool[node_id].key_sign, node->key_sign);
		if (cmp == 0) 
		{
            printf ("key already exist");
			return -1;
		} else if (cmp < 0) 
		{
			pre_node_ptr = &(idx->node_pool[node_id].left_id);
			node_id = idx->node_pool[node_id].left_id;
		} else 
		{
			pre_node_ptr = &(idx->node_pool[node_id].right_id);
			node_id = idx->node_pool[node_id].right_id;
		}
	}

	new_node_id = _get_free_idx_node(idx);
	if (new_node_id == -1)
	{
        printf ("no free idx node\n");
		return -1;
	}
	idx->node_pool[new_node_id].key_sign = node->key_sign;
    idx->node_pool[new_node_id].value_offset = node->value_offset;
	idx->node_pool[new_node_id].left_id = INDEX_NODE_NOT_EXIST;
    idx->node_pool[new_node_id].right_id = INDEX_NODE_NOT_EXIST;
	*pre_node_ptr = new_node_id;	

    idx->timestamp = timestamp;
    idx->is_change = 1;
	return 0;
}

int idx_delete(INDEX* idx, IDX_NODE* node, unsigned long long timestamp)
{
    int cmp = 0;
	unsigned long long node_id = 0;
    unsigned long long nearest_id = 0;
	unsigned long long ht_table_id = 0;
	unsigned long long* pre_node_ptr = NULL;
	unsigned long long* nearest_pre_node_ptr = NULL;
	
    ht_table_id = _get_hashhead_id(node->key_sign, idx->hashhead_size); 
	pre_node_ptr = &(idx->hash_head[ht_table_id]);
	node_id = idx->hash_head[ht_table_id];

	while ( 1 )
	{
		if (node_id == INDEX_NODE_NOT_EXIST)
		{
            printf ("delete key not found");
			return -1;
		}

		cmp = _is_hash_same(idx->node_pool[node_id].key_sign, node->key_sign);
		if (cmp == 0) {
			break;
		} else if (cmp < 0) {
			pre_node_ptr = &(idx->node_pool[node_id].left_id);
			node_id = idx->node_pool[node_id].left_id;
		} else {
			pre_node_ptr = &(idx->node_pool[node_id].right_id);
			node_id = idx->node_pool[node_id].right_id;
		}
	}

    //delete needn't
    node->value_offset = idx->node_pool[node_id].value_offset;

	if (idx->node_pool[node_id].left_id == INDEX_NODE_NOT_EXIST
        && idx->node_pool[node_id].right_id == INDEX_NODE_NOT_EXIST) {
		*pre_node_ptr = INDEX_NODE_NOT_EXIST;
	} else if (idx->node_pool[node_id].left_id != INDEX_NODE_NOT_EXIST
        && idx->node_pool[node_id].right_id != INDEX_NODE_NOT_EXIST) {
		nearest_pre_node_ptr = &(idx->node_pool[node_id].right_id);
		nearest_id = idx->node_pool[node_id].right_id;
		while (idx->node_pool[nearest_id].left_id != INDEX_NODE_NOT_EXIST) {
			nearest_pre_node_ptr = &(idx->node_pool[nearest_id].left_id);
			nearest_id = idx->node_pool[nearest_id].left_id;
		}

		*nearest_pre_node_ptr = idx->node_pool[nearest_id].right_id;
		idx->node_pool[nearest_id].left_id = idx->node_pool[node_id].left_id;
		idx->node_pool[nearest_id].right_id = idx->node_pool[node_id].right_id;
		*pre_node_ptr = nearest_id;
	} else if (idx->node_pool[node_id].left_id != INDEX_NODE_NOT_EXIST) {
		*pre_node_ptr = idx->node_pool[node_id].left_id;		
	} else {
		*pre_node_ptr = idx->node_pool[node_id].right_id;
	}

	_put_free_idx_node(idx, node_id);

    idx->timestamp = timestamp;
    idx->is_change = 1;
	return 0;
}

static int _is_hash_same(unsigned long long a, unsigned long long b)
{
    if (a == b) {
        return 0;
    }
    if (a < b) {
        return -1;
    }
    return 1;
}

static unsigned long long _get_free_idx_node(INDEX *idx)
{
    unsigned long long ret = 0;
    /*TO DO if xx < 0 */
    if (0 == idx->free_node_count) {
        ret = idx->node_pool_horizon;
        ++(idx->node_pool_horizon);
    } else {
        ret = idx->free_node_pool[idx->free_node_count];
        --(idx->free_node_count);
    }
	return ret;
}

static void _put_free_idx_node(INDEX* idx, unsigned long long node_id)
{
    /*TO DO : some check */
	idx->free_node_pool[++(idx->free_node_count)] = node_id;
}

static INDEX_HEAD_LAYOUT _indexhead_fetch(INDEX* index)
{
    INDEX_HEAD_LAYOUT indexhead;
    indexhead.magic_num = 1806;
    indexhead.version = 0;
    indexhead.cur_kv_count = index->cur_kv_count;
    indexhead.node_pool_size = index->node_pool_size;
    indexhead.hash_head = index->hashhead_size;
    indexhead.free_node_pool_size = index->free_node_pool_size;
    indexhead.free_node_count = index->free_node_count;
    indexhead.timestamp = index->timestamp;
    indexhead.node_pool_horizon = index->node_pool_horizon;
    return indexhead;
}

static INDEX_HEAD_LAYOUT _indexhead_read(int fd)
{
    INDEX_HEAD_LAYOUT indexhead;
    lseek(fd, 0, SEEK_SET);
    read(fd, &indexhead, sizeof(indexhead));
    return indexhead;
}

static INDEX_HEAD_LAYOUT _indexhead_create(unsigned long long key_num_quotas, unsigned long long timestamp)
{
    INDEX_HEAD_LAYOUT indexhead;
    indexhead.magic_num = 1806;
    indexhead.version = 0;
    indexhead.cur_kv_count = 0;
    indexhead.node_pool_size = key_num_quotas;
    indexhead.hash_head = THIS_IS_HASH_HEAD_NUM;
    indexhead.free_node_pool_size = THIS_IS_FREE_NODE_POOL_SIZE;
    indexhead.free_node_count = 0;
    indexhead.timestamp = timestamp;
    indexhead.node_pool_horizon = 0;
    return indexhead;
}
