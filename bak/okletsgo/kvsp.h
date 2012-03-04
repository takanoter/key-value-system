/* Some packets:
 * client send request:
 * 0. | KVSP_REQ_GET | SEQ_ID | KEY_LEN | ... |
 * 1. | KVSP_REQ_DEL | SEQ_ID | KEY_LEN | ... |
 * 2. | KVSP_REQ_PUT | SEQ_ID | KEY_LEN | VALUE_LEN | ... | ... |
 *
 * client recv answer:
 * 3. | KVSP_ANS_GET_YES | SEQ_ID | VALUE_LEN | ... |
 * 4. | KVSP_ANS_GET_NO  | SEQ_ID |
 * 5. | KVSP_ANS_PUT_YES | SEQ_ID |
 * 6. | KVSP_ANS_PUT_NO  | SEQ_ID |
 * 7. | KVSP_ANS_DEL_YES | SEQ_ID |
 * 8. | KVSP_ANS_DEL_NO  | SEQ_ID |
 */ 

/*FIXME: same as packet.h
 */
#ifndef KVS_PACKET_TYPE
#define	KVS_PACKET_TYPE
	#define	KVSP_REQ_GET		0
	#define	KVSP_REQ_DEL		1
	#define	KVSP_REQ_PUT		2
	#define	KVSP_REQ_CLOSE		3

	#define	KVSP_ANS_GET_YES	13	
	#define	KVSP_ANS_GET_NO		14
	#define	KVSP_ANS_PUT_YES	15
	#define	KVSP_ANS_PUT_NO		16
	#define	KVSP_ANS_DEL_YES	17
	#define	KVSP_ANS_DEL_NO		18
#endif


struct KV_RECV_HEAD
{
	int type;
	int seq_id;
};

struct KV_SEND_HEAD
{
	int type, seq_id, key_len, value_len;
	char* key;
	char* value;
};

/* return data-size to be read if necessary.
 */
int client_recv_head(int fd, struct KV_RECV_HEAD * kv_recv_head);
void client_recv_date(int fd, int size, char* buf, int buf_size);
	
int client_send(int fd, struct KV_SEND_HEAD* kv_send_head);

void client_close(int fd);


