#include <stdio.h>
#include <stdlib.h>
#include "../kvs_api.h"
#include "../kvs_type.h"
/*
int kv_open(KVS **kvs, const char *dbfile, KVS_OPT *option);
int kv_close(KVS **kvs);
int kv_put(KVS *kvs, void* key, int key_len, void* value, int value_len);
int kv_get(KVS *kvs, KV_PAIR *kv_pair);
int kv_delete(KVS *kvs, const char *key, int key_len);
int kv_sync(KVS *kvs, int sync_option);
*/
static void test_insert(KVS* kvs, void *key, int key_len);
static void test_delete(KVS* kvs, void *key, int key_len);
static void test_select(KVS* kvs, void *key, int key_len);
int main()
{
    int ret = 0;
    KVS* kvs = NULL;
    KVS_OPT kvs_option;
    kvs_option.key_num = 10000;
    kvs_option.open_mode = CREATE;
    kvs_option.sync_mode = INDEX_KEEP_SYNC;

    ret = kv_open(&kvs, "dbfile", &kvs_option); 

    test_insert(kvs, "haha", sizeof("haha"));
    test_select(kvs, "haha", sizeof("haha"));
    test_insert(kvs, "hehe", sizeof("hehe"));
    test_insert(kvs, "hhhh", sizeof("hhhh"));
    test_select(kvs, "hehe", sizeof("haha"));
    test_select(kvs, "haha", sizeof("haha"));

    kv_close(&kvs);

    return 0;
}

static void test_select(KVS* kvs, void *key, int key_len)
{
    int i;
    KV_PAIR kv_pair;
    kv_pair.timestamp = 0;
    kv_pair.key_len = key_len;
    kv_pair.value_len = 0;
    kv_pair.key = key;
    kv_pair.value = NULL;
    kv_get(kvs, &kv_pair);

    printf ("key:%s   value:", (char*)key);
    for (i=0; i<kv_pair.value_len; i++)
    {
        printf ("[%c]", ((char*)kv_pair.value)[i]);
    }
    printf ("\n\n");
}

static void test_insert(KVS* kvs, void *key, int key_len)
{
    kv_put(kvs, key, key_len, key, key_len);
}

static void test_delete(KVS* kvs, void *key, int key_len)
{
    return;
}

