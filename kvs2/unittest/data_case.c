#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../data.h"
#include "../kvs_type.h"

/*
typedef struct DATA {
    int fd;
    unsigned long long append_offset;
    unsigned long long kv_count;
    unsigned long long timestamp;
} DATA;
*/
static void test_delete(DATA* data, unsigned long long timestamp);
static void test_select(DATA* data, char* key, unsigned long long key_len, unsigned long long offset);
static void test_insert(DATA* data, unsigned long long timestamp, char* key, unsigned long long key_len, unsigned long long offset);

int main()
{
    DATA *data = NULL;   

    //test case for data_create
/*
    printf ("test case for idx_create() ********************************\n");
    data = data_create("data_testfile");
    if (NULL == data) {
        printf ("data is null.\n");
        return -1;
    }
    printf ("append_offset:%llu\tkv_count:%llu\ttimestamp:%llu\n",
            data->append_offset, data->kv_count, data->timestamp);
    data_exit(data);
    data = NULL;
    printf ("***********************************************************\n\n\n");
*/

    //test case for data_load
    printf ("test case for data_load() ********************************\n");
    data = data_load("data_testfile");
    if (NULL == data) {
        printf ("data is null.\n");
        return -1;
    }
    printf ("append_offset:%llu\tkv_count:%llu\ttimestamp:%llu\n",
            data->append_offset, data->kv_count, data->timestamp);
    printf ("***********************************************************\n\n\n");

    //test case for insert
/*
    printf ("test case for insert ********************************\n");
    test_insert(data, 1102, "hello", sizeof("hello"), 1000);
    printf ("***********************************************************\n\n\n");
*/

    //test case for select
    printf ("test case for select ********************************\n");
    test_select(data, "hello", sizeof("hello"), 1000);
    printf ("***********************************************************\n\n\n");

    //test case for delete
    printf ("test case for delete ********************************\n");
    test_delete(data, 1112);
    printf ("***********************************************************\n\n\n");


    data_exit(data);
    return 0;
}

/*
typedef struct KV_PAIR{
    unsigned long long timestamp;    
    unsigned long long key_len;
    unsigned long long value_len;
    void* key;
    void* value;
} KV_PAIR;
*/
static void test_insert(DATA* data, unsigned long long timestamp, char* key, unsigned long long key_len, unsigned long long offset)
{
    int ret = 0;
    KV_PAIR kv_pair;
    char buffer_key[1024];
    char buffer_value[1024];

    memset(buffer_key, 0, sizeof(buffer_key));
    memset(buffer_value, 0, sizeof(buffer_value));
    kv_pair.timestamp = timestamp;
    kv_pair.key_len = key_len;
    kv_pair.value_len = key_len;
    sprintf(buffer_key, "%s", key);
    sprintf(buffer_value, "%s", key);
    kv_pair.key = buffer_key;
    kv_pair.value = buffer_value;

    ret = data_insert(data, offset, &kv_pair);
    if (0 != ret) {
        printf ("ret error.\n");
        return;
    }
    printf ("timestamp:%llu\tkey[%s]\toffset:%llu\n", timestamp, key, offset);
    
    return;
}

static void test_select(DATA* data, char* key, unsigned long long key_len, unsigned long long offset)
{
    int ret = 0;
    KV_PAIR kv_pair;
    char buffer_key[1024];
    char buffer_value[1024];
    memset(buffer_key, 0, sizeof(buffer_key));
    memset(buffer_value, 0, sizeof(buffer_value));
    
    kv_pair.timestamp = 1111;
    kv_pair.key_len = 0;
    kv_pair.value_len = 0;
    kv_pair.key = NULL;
    kv_pair.value = NULL;

    ret = data_get(data, offset, &kv_pair);
    if (0 != ret) {
        printf ("ret error.\n");
        return;
    }

    printf("select: key:[%s]\tvalue[%s]\n", (char*)kv_pair.key, (char*)kv_pair.value);
    return;
}

static void test_delete(DATA* data, unsigned long long timestamp)
{
    int ret = 0;
    ret = data_delete(data, timestamp);
    if (0 != ret) {
        printf ("ret error.\n");
        return;
    }
    return;
}

