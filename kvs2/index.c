#include <stdio.h>
#include <errno.h>

#include "index.h"

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
    return indexhead;
};

int index_head_sync(INDEX* index)
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
    return 0;
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
    return indexhead;
}

static INDEX_HEAD_LAYOUT _indexhead_read(int fd)
{
    INDEX_HEAD_LAYOUT indexhead;
    lseek(fd, 0, SEEK_SET);
    read(fd, &indexhead, sizeof(indexhead));
    return indexhead;
}

INDEX* idx_create(const char *filename, unsigned long long key_num_quotas)
{
    INDEX* index = NULL;
    int fd = -1;

	fd = open(filename, O_RDWR | O_LARGEFILE | O_CREAT , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); 
	if (fd <= 0) 
	{
        printf ("open [%s] failed.\n");
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
    index->hash_head = (unsigned long long*) malloc(sizeof(*hash_head) * indexhead.hash_head);
    for (i = 0; i< index->hashhead_size; i++) {
        index->hash_head[i] = INDEX_NODE_NOT_EXIST;
    }
    index->node_pool_size = indexhead.node_pool_size;
    index->node_pool = (IDX_NODE*) malloc(sizeof(*node_pool) * indexhead.node_pool_size);
    for (i = 0; i < index->node_pool_size; i++) {
        index->node_pool[i] = INDEX_NODE_NOT_EXIST;
    }
    index->free_node_pool_size = indexhead.free_node_pool_size;
    index->free_node_count = indexhead.free_node_count;
    index->free_node_pool = (unsigned long long*) malloc(sizeof(*(index->free_node_pool) * indexhead.free_node_pool_size));
    index->timestamp = indexhead.timestamp;
    
    ret = index_head_sync(index);
    if (err_success != ret) {
        goto OUT;
    }

    return index;

OUT:
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

	fd = open(filename, O_RDWR | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); 
	if (fd <= 0) {
        printf ("load [%s] failed.\n");
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
    index->hash_head = (unsigned long long*) malloc(sizeof(*hash_head) * indexhead.hash_head);
    index->node_pool_size = indexhead.node_pool_size;
    index->node_pool = (IDX_NODE*) malloc(sizeof(*node_pool) * indexhead.node_pool_size);
    index->free_node_pool_size = indexhead.free_node_pool_size;
    index->free_node_count = indexhead.free_node_count;
    index->free_node_pool = (unsigned long long*) malloc(sizeof(*(index->free_node_pool) * indexhead.free_node_pool_size));
    index->timestamp = indexhead.timestamp;

    lseek(index->fd, sizeof(INDEX_HEAD_LAYOUT), SEEK_SET);
    read(index->fd, index->node_pool,
            index->node_pool_size * sizeof(*(index->node_pool)) );
    read(index->fd, index->hash_head,
            index->hashhead_size * sizeof(*(index->hash_head)) );
    read(index->fd, index->free_node_pool,
            index->free_node_pool_size * sizeof(*(index->free_node_pool)) );
    
    return index;

OUT:
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




int idx_insert(INDEX* idx, const IDX_NODE* node, unsigned long long timestamp);
{
	unsigned long long ht_table_id; 
    unsigned long long node_id, new_node_id;
    int cmp = 0;
	unsigned long long*	pre_node_ptr;

    int ht_table_id = _get_hashhead_id(node->key_sign);
	
	pre_node_ptr = &idx->hash_head[ht_table_id];
	node_id = ht_table[ht_table_id];

	while (node_id != INDEX_NODE_NOT_EXIST)
	{
		cmp = _is_hash_same(idx->node_pool[node_id], node->key_sign);
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
	return 0;
}

typedef struct INDEX {
    FILE* fd;
	int is_change;
    int cur_kv_count;

	unsigned long long hashhead_size;
	unsigned long long* hash_head;

	/*this is max_key_num*/
	unsigned long long node_pool_size; 
	IDX_NODE* node_pool;

    unsigned long long free_node_pool_size;
	unsigned long long free_node_count;
	unsigned long long* free_node_pool;

    unsigned long long timestamp;
} INDEX;


int idx_search(INDEX* idx, IDX_NODE* node)
{
	unsigned long long ht_table_id; 
    unsigned long long node_id, new_node_id;
    int cmp = 0;
	unsigned long long*	pre_node_ptr;
    int ht_table_id = _get_hashhead_id(node->key_sign);

	(*search_node)->buf_ptr = BUF_PTR_NULL;
	(*search_node)->disk_offset = DISK_OFFSET_NULL;
	(*search_node)->value_size = -1;

	cur_hash_2 = hash_func_2(key, key_size);
	cur_hash_3 = hash_func_3(key, key_size);

	node_id = ht_table[ht_table_id];

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

typedef struct IDX_NODE 
{
	IDX_VALUE_INFO			value_node;
	PTR_KW					left_id, right_id;
	HASH					hash_2, hash_3;
}IDX_NODE;

typedef struct IDX_NODE {
    unsigned long long key_sign;
    unsigned long long value_offset;
    unsigned long long left_id;
    unsigned long long right_id;
}
int idx_delete(const char* key, int key_size, IDX_VALUE_INFO* delete_node)
int idx_delete(INDEX* idx, const IDX_NODE* node, unsigned long long timestamp)
{
	unsigned long long node_id, nearest_id;
    int cmp;
	unsigned long long* pre_node_ptr = NULL;
	unsigned long long* nearest_pre_node_ptr = NULL;
	
	int ht_table_id = _get_hashhead_id(node->key_sign); 
	pre_node_ptr = &ht_table[ht_table_id];
	node_id = ht_table[ht_table_id];

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
			node_id = &(idx->node_pool[node_id].left_id);
		} else {
			pre_node_ptr = &(idx->node_pool[node_id].right_id);
			node_id = &(idx->node_pool[node_id].right_id);
		}
	}

/*
	delete_node->buf_ptr = idx_nodes[node_id].value_node.buf_ptr;
	delete_node->disk_offset = idx_nodes[node_id].value_node.disk_offset;
	delete_node->value_size = idx_nodes[node_id].value_node.value_size;
*/

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
	return 0;
}

int idx_exit()
{
	if (log_file)
		log_err(__FILE__, __LINE__, log_file, "INDEX EXIT.");

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

typedef struct INDEX {
    FILE* fd;
	int is_change;
    int cur_kv_count;

	unsigned long long hashhead_size;
	unsigned long long* hash_head;

	/*this is max_key_num*/
	unsigned long long node_pool_size; 
	IDX_NODE* node_pool;

    unsigned long long free_node_pool_size;
	unsigned long long free_node_count;
	unsigned long long* free_node_pool;

    unsigned long long timestamp;
} INDEX;
static unsigned long long _get_free_idx_node(INDEX *idx)
{
    /*TO DO if xx < 0 */
	return idx->free_node_pool[(idx->free_node_count)--];
}

static void _put_free_idx_node(INDEX* idx, unsigned long long node_id)
{
	idx->free_node_pool[++(idx->free_node_count)] = node_id;
}
