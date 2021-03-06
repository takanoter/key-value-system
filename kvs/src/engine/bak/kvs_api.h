#ifndef KVS_LIB_API
#define KVS_LIB_API

#define KVS_MAX_NAME 1024
#define KVS_USER_SIZE 1024
#include "data.h"
#include "index.h"
/**
 * KVS LIB
 */
enum open_mode_t {
    CREATE = 1, 
    LOAD = 2,
};

enum sync_mode_t {
    INDEX_KEEP_SYNC = 1,
    DATA_HEAD_KEEP_SYNC = 2,
};

enum sync_option_t {
    INDEX_SYNC = 1,
    DATA_HEAD_SYNC = 2,
    DIRECT_IO = 4,
};

typedef struct KVS_OPT {
    /*upper layer should weihu data*/
    unsigned long long key_num; /*TODO, only available in opne_mode=CREATE now*/

    /*base operations*/
    enum open_mode_t    open_mode;
    enum sync_mode_t    sync_mode;

    /*third component*/
    char           log_filename[KVS_MAX_NAME];/*FIXME*/
    /*TODO: add third malloc, free  or memory pool */
} KVS_OPT;

typedef struct KVS {
    /* KVS configure operations */
    char    idx_name[KVS_MAX_NAME];
    char    data_name[KVS_MAX_NAME];
    KVS_OPT option;

    /* KVS status memory */
    INDEX* idx;
    DATA* data;

    /* other informations */
    char  user_specify_info[KVS_USER_SIZE];
} KVS;



int kv_open(KVS **kvs, const char *dbfile, KVS_OPT *option);
int kv_close(KVS **kvs);

/*KVS opeations*/
int kv_put(KVS *kvs, void* key, int key_len, void* value, int value_len);
int kv_get(KVS *kvs, KV_PAIR *kv_pair);
int kv_delete(KVS *kvs, const char *key, int key_len);
int kv_sync(KVS *kvs, int sync_option);

#endif
