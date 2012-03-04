/* FIXME: This is KVS Library Header file: kvs.h
 * Programmer can get help from this header file.
 * To write "seems-good" comment of interface, 
 * learn from other open-source document.
 */

#define	KVS_INIT_SUCCESS		0x00000000
#define	KVS_INIT_NG_BIG_FILE		0xffff000f
#define	KVS_INIT_NG_IDX_LOG		0xffff00f0
#define	KVS_INIT_NG_DISK_LOG		0xffff0f00
#define	KVS_INIT_NG_SYNC		0xfffff000
#define	KVS_INIT_NG_MEMORY_LIMIT	0xffff00ff

#define	KVS_EXIT_SUCCESS		0x00000000
#define	KVS_EXIT_NG_SYNC		0xffff000f

#define	KVS_GET_SUCCESS			0x00000000
#define	KVS_GET_NG_NOT_EXIST		0xffff000f 
#define	KVS_GET_NG_SYNC			0xffff00f0
#define	KVS_GET_NG_INNER		0xffff0f00
#define	KVS_GET_NG_MEMORY		0xfffff000

#define	KVS_PUT_SUCCESS			0x00000000
#define	KVS_PUT_NG_EXIST		0xffff000f
#define	KVS_PUT_NG_VALUE_SIZE		0xffff00f0
#define	KVS_PUT_NG_DISK_SPACE_OUT	0xffff0f00
#define	KVS_PUT_NG_SYNC			0xfffff000

#define	KVS_DELETE_SUCCESS		0x00000000
#define	KVS_DELETE_NG_DISK_FREE		0xffff000f
#define	KVS_DELETE_NG_NOT_EXIST		0xffff00f0

#define	KVS_CREATE			0
#define	KVS_LOAD			1

struct KVS_ENV
{
	int	init_type;
	int 	size_in_g;
	char*	disk_file;
	char*	disk_log;
	char*	sync_log;
	char*	index_log;
};

struct KV_IDX
{
	int	disk_location;
	int	value_size;
};

/*XXX: these interfaces are
 * 1.Blocking -- May cause Caller lower performance
 * 2.Not Thread Safe --  May cause Caller Layer complex design
 */
int	kv_init(const struct KVS_ENV* kvs);
int	kv_exit();
int	kv_get(const char* key, int key_size, char* buf, int buf_size);
int	kv_put(const char* key, int key_size, const char* value, int value_size);
int	kv_delete(const char* key, int key_size);
void	kv_image_flush();

int	kv_get_index(const char* key, int key_size, struct KV_IDX* kv_idx);
int	kv_read_index(const struct KV_IDX* kv_idx, char* buf, int buf_size);

