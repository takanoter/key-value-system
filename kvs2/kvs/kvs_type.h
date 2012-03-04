#ifndef KVS_LIB_TYPE
#define KVS_LIB_TYPE 1

typedef struct KV_PAIR{
    unsigned long long timestamp;    
    unsigned long long key_len;
    unsigned long long value_len;
    void* key;
    void* value;
} KV_PAIR;

#define err_success             0
#define err_input_param_null    -1
#define err_malloc_failed       -2
#define err_init_index          -3
#define err_init_data           -4

#endif
