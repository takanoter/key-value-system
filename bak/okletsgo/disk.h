#ifndef KVS_DISK
#define KVS_DISK

#define	DISK_SUCCESS			0x00000000
#define	DISK_NG_INIT_LOG		0xffff000f
#define	DISK_NG_SLOT_CNT		0xffff00f0
#define	DISK_NG_LIMIT_OUT		0xffff0f00
#define	DISK_NG_FREE			0xfffff000

STATE	disk_init(INIT_TYPE init_type, char* image, const char* disk_log_file, const int disk_size_in_chunk);
void	disk_exit();
int	disk_alloc(int);
STATE	disk_free(int);

#endif

