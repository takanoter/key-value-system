#include <stdio.h>
#include "kvs_utils.h"

/* These info will persist */
typedef struct INDEX_HEAD_LAYOUT {
    int magic_num;
	int version;
	unsigned long long cur_kv_count;
	unsigned long long node_pool_size;
    unsigned long long node_pool_horizon;
	unsigned long long hash_head; /* hash_head slot count */

	unsigned long long free_node_pool_size;
    unsigned long long free_node_count;

    unsigned long long timestamp;
	
	/*node_pool;*/
	/*hash_head;*/
	/*free_node_pool;*/
} INDEX_HEAD_LAYOUT;

typedef struct IDX_NODE {
    unsigned long long key_sign;
    unsigned long long value_offset;
    unsigned long long left_id;
    unsigned long long right_id;
} IDX_NODE;

typedef struct INDEX {
    int fd;
	int is_change;
    int cur_kv_count;

	unsigned long long hashhead_size;
	unsigned long long* hash_head;

	/*this is max_key_num*/
	unsigned long long node_pool_size; 
    unsigned long long node_pool_horizon;
	IDX_NODE* node_pool;

    unsigned long long free_node_pool_size;
	unsigned long long free_node_count;
	unsigned long long* free_node_pool;

    unsigned long long timestamp;
} INDEX;



INDEX* idx_create(const char *filename, unsigned long long key_num);
INDEX* idx_load(const char *filename);
int idx_exit(INDEX *index);
int idx_sync(INDEX* index);

int idx_insert(INDEX* idx, const IDX_NODE* node, unsigned long long timestamp);
int idx_delete(INDEX* idx, IDX_NODE* node, unsigned long long timestamp);
int idx_search(INDEX* idx, IDX_NODE* node);
