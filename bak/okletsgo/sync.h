#ifndef KVS_SYNC
#define KVS_SYNC

#define	SYNC_SUCCESS		0x00000000
#define	SYNC_NG_FD		0xffff000f
#define	SYNC_NG_INIT_LOG	0xffff00f0
#define	SYNC_NG_LSEEK		0xffff0f00

STATE sync_write(PTR_DISK _disk, const char* _mem, int size);
STATE sync_read(PTR_DISK _disk, char* _mem, int size);
STATE sync_init(char *pathname, char*);
void  sync_exit();

#endif
