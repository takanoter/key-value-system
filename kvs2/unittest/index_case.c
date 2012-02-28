#include <stdio.h>
#include <stdlib.h>
#include "../index.h"
/*
typedef struct INDEX {
    int fd;
	int is_change;
    int cur_kv_count;

	unsigned long long hashhead_size;
	unsigned long long* hash_head;

	unsigned long long node_pool_size; 
	IDX_NODE* node_pool;

    unsigned long long free_node_pool_size;
	unsigned long long free_node_count;
	unsigned long long* free_node_pool;

    unsigned long long timestamp;
} INDEX;
*/
static void test_insert(INDEX* idx, unsigned long long timestamp, unsigned long long key_sign);
static void test_delete(INDEX* idx, unsigned long long timestamp, unsigned long long key_sign);
static void test_select(INDEX* idx, unsigned long long key_sign);
int main()
{
    INDEX *idx = NULL;   
    //test case for idx_create
/*
    printf ("test case for idx_create() ********************************\n");
    idx = idx_create("index_testfile", 2000);
    if (NULL == idx) {
        printf ("idx_fd is null.\n");
        return -1;
    }
    printf("is_change:%d\ncur_kv_count:%d\nhashhead_size:%llu\nnode_pool_size:%llu\n",
            idx->is_change, idx->cur_kv_count, idx->hashhead_size, idx->node_pool_size);
    printf("free_node_pool_size:%llu\nfree_node_count:%llu\ntimestamp:%llu\n",
            idx->free_node_pool_size, idx->free_node_count, idx->timestamp);
    if (NULL == idx->hash_head || NULL == idx->node_pool || NULL == idx->free_node_pool) {
        printf ("idx some struct is NULL.\n");
        return -1;
    }
    idx_exit(idx);
    idx = NULL;
*/
    printf ("***********************************************************\n\n\n");

    //test case for idx_load
    printf ("test case for idx_load() ********************************\n");
    idx = idx_load("index_testfile");
    if (NULL == idx) {
        printf ("idx_fd is null.\n");
        return -1;
    }
    printf("is_change:%d\ncur_kv_count:%d\nhashhead_size:%llu\nnode_pool_size:%llu\n",
            idx->is_change, idx->cur_kv_count, idx->hashhead_size, idx->node_pool_size);
    printf("free_node_pool_size:%llu\nfree_node_count:%llu\ntimestamp:%llu\n",
            idx->free_node_pool_size, idx->free_node_count, idx->timestamp);
    if (NULL == idx->hash_head || NULL == idx->node_pool || NULL == idx->free_node_pool) {
        printf ("idx some struct is NULL.\n");
        return -1;
    }
    printf ("***********************************************************\n\n\n");

    //test case for insert
    printf ("test case for insert ********************************\n");
    test_insert(idx, 1101, 1806);
    test_insert(idx, 1102, 1807);
    test_insert(idx, 1102, 1808);
    test_insert(idx, 1102, 1807);
    printf ("***********************************************************\n\n\n");

    //test case for select
    printf ("test case for select ********************************\n");
    test_select(idx, 1806);
    test_select(idx, 1807);
    test_select(idx, 1806);
    test_select(idx, 1806);
    test_select(idx, 1805);
    test_select(idx, 1808);
    printf ("***********************************************************\n\n\n");

    //test case for delete
    printf ("test case for delete ********************************\n");
    test_delete(idx, 1111, 1806);
    test_delete(idx, 1112, 1806);
    printf ("***********************************************************\n\n\n");


    idx_exit(idx);
    return 0;
}

/*
typedef struct IDX_NODE {
    unsigned long long key_sign;
    unsigned long long value_offset;
    unsigned long long left_id;
    unsigned long long right_id;
} IDX_NODE;
*/
static void test_insert(INDEX* idx, unsigned long long timestamp, unsigned long long key_sign)
{
    int ret = 0;
    struct IDX_NODE idx_node;
    idx_node.key_sign = key_sign;
    idx_node.value_offset = key_sign / 2;
    ret = idx_insert(idx, &idx_node, timestamp);
    if (0 != ret) {
        printf ("ret error.\n");
        return;
    }
    printf("insert: key_sign:%llu\tvalue_offset:%llu\ttimestamp:%llu\n", key_sign, idx_node.value_offset, timestamp);
    return;
}

static void test_select(INDEX* idx, unsigned long long key_sign)
{
    int ret = 0;
    struct IDX_NODE idx_node;
    idx_node.key_sign = key_sign;
    idx_node.value_offset = 1111;
    ret = idx_search(idx, &idx_node);
    if (0 != ret) {
        printf ("ret error.\n");
        return;
    }
    printf("select: key_sign:%llu\tvalue_offset:%llu\n", key_sign, idx_node.value_offset);
    return;
}

static void test_delete(INDEX* idx, unsigned long long timestamp, unsigned long long key_sign)
{
    int ret = 0;
    struct IDX_NODE idx_node;
    idx_node.key_sign = key_sign;
    idx_node.value_offset = 1111;
    ret = idx_delete(idx, &idx_node, timestamp);
    if (0 != ret) {
        printf ("ret error.\n");
        return;
    }
    printf("delete: key_sign:%llu\tvalue_offset:%llu\ttimestamp:%llu\n", key_sign, idx_node.value_offset, timestamp);
}

