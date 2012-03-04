#define VALUE_TYPE_INT  1

typedef struct KVS_PACK {
    int *send_length;
    int buf_size;
    char *buf; 
} KVS_PACK;

KVS_PACK * kvsp_create(void* buf, int buf_len);

KVS_PACK * kvsp_load(void* buf, int buf_size);

int kvsp_destroy(struct KVS_PACK* pack);

int kvsp_put(KVS_PACK* pack, void* name, int name_len, int type, void* value, int value_len);

int kvsp_get(KVS_PACK* pack, const void* name, int name_len, int* type, void** value, int *value_len);

void* kvsp_getbuf(KVS_PACK* pack);

int kvsp_get_sendlen(KVS_PACK* pack);

void kvsp_check(KVS_PACK *pack);

