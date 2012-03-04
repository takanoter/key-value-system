/* index.h */

#define	IDX_SUCCESS		0x00000000
#define	IDX_NG_INIT_LOG		0xffff000f
#define	IDX_NG_NOT_EXIST	0xffff00f0
#define	IDX_NG_EXIST		0xffff0f00

STATE	idx_init(INIT_TYPE init_type, const char* g_image, const char* index_log);
STATE	idx_search(const char* key, int key_len, PTR_DISK* disk_location, int* value_len);
STATE	idx_insert(const char* key, int key_len, PTR_DISK disk_location, int value_len);
STATE	idx_delete(const char* key, int key_len, PTR_DISK* disk_location);
void	idx_show();
void	idx_exit();

