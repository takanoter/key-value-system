/* These info will persist */
typedef struct DATA{
    int fd;
    unsigned long long append_offset;
    unsigned long long kv_count;
    unsigned long long timestamp
} DATA;

DATA* data_create(const char* head_filename);
DATA* data_load(const char* head_file_name);

int data_insert(DATA* data, unsigned long long value_offset, KV_PAIR* kv_pair);
int data_delete(DATA* data, unsigned long long timestamp);
int data_get(DATA* data, unsigned long long value_offset, KV_PAIR* kv_pair); 

int data_head_sync(DATA* data);
