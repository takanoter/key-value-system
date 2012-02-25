/* These info will persist */
typedef struct INDEX_HEAD_LAYOUT {
	unsigned long long size;
	int version;
	unsigned long long node_pool_size;
	unsigned long long hash_head;
	unsigned long long free_node_pool_size;
    unsigned long long timestamp;
	
	/*node_pool;*/
	/*hash_head;*/
	/*free_node_pool;*/
} INDEX_HEAD_LAYOUT;

typedef struct IDX_NODE {
    unsigned long long key_sign;
    unsigned long long value_offset;
    unsigned long long left_node_pool_id;
    unsigned long long right_node_pool_id;
}

typedef struct INDEX {
    FILE* fd;
	int is_change;

	unsigned long long hash_head;
	void* hash_head;

	/*this is max_key_num*/
	unsigned long long node_pool_count; 
	IDX_NODE* node_pool;

	unsigned long long free_node_count;
	int* free_node_pool;
} INDEX;



INDEX* idx_create(const char *head_filename, unsigned long long key_num);
INDEX* idx_load(const char *head_filename);

int idx_insert(INDEX* idx, const IDX_NODE* node, unsigned long long timestamp);
int idx_delete(INDEX* idx, const IDX_NODE* node, unsigned long long timestamp);
int idx_search(INDEX* idx, IDX_NODE* node);
