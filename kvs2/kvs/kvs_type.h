#ifndef KVS_LIB_TYPE
#define KVS_LIB_TYPE

typedef struct KV_PAIR{
    unsigned long long timestamp;    
    unsigned long long key_len;
    unsigned long long value_len;
    void* key;
    void* value;
} KV_PAIR;


#endif
