#include "kvs_type.h"
#include "kvs_utils.h"
/* These info will persist */
typedef struct DATA {
    int fd;
    unsigned long long append_offset;
    unsigned long long kv_count;
    unsigned long long timestamp;
} DATA;

int data_head_sync(DATA* data);
unsigned long long data_get_location(DATA* data, unsigned long long non_use_len);

DATA* data_create(const char* head_filename);
DATA* data_load(const char* head_file_name);
int data_exit(DATA* data);

int data_get(DATA* data, unsigned long long value_offset, KV_PAIR* kv_pair); 
int data_insert(DATA* data, unsigned long long value_offset, KV_PAIR* kv_pair);
int data_delete(DATA* data, unsigned long long timestamp);



