#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Some packets:
 * 0. | KVSP_REQ_GET | SEQ_ID | KEY_LEN | ... |
 * 1. | KVSP_REQ_DEL | SEQ_ID | KEY_LEN | ... |
 * 2. | KVSP_REQ_PUT | SEQ_ID | KEY_LEN | VALUE_LEN | ... | ... |
 * 3. | KVSP_ANS_GET_YES | SEQ_ID | VALUE_LEN | ... |
 * 4. | KVSP_ANS_GET_NO  | SEQ_ID |
 * 5. | KVSP_ANS_PUT_YES | SEQ_ID |
 * 6. | KVSP_ANS_PUT_NO  | SEQ_ID |
 * 7. | KVSP_ANS_DEL_YES | SEQ_ID |
 * 8. | KVSP_ANS_DEL_NO  | SEQ_ID |
 */ 

/*FIXME: same as client_func.h
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

/* Analyse & Design:
 * 1.Logic Model
 * 2.State Machine  (state machine is NOT necessary now.)
 * 3.Packet Format Design (can add packet.parse() when without State Machine)
 *
 * Questions:
 * 1. EpollIN or just Connection Oriented ?
 * 2. How to organize "packet format" & "state machine" ? in the same struct ?
 * 3. When to close Epoll_fd ?  (server how to close connection? need state machine? )
 */

/*XXX: is "seq_id" necessary ?
 * Using TCP:
 * Client send PACKET_seq_0, then PACKET_seq_1, 
 * Server may recv PACKET_seq_1, then PACKET_seq_0.
 *XXX: non blocking in RECV and SEND ?
 */

/* 0, 1, 2, 3
 */
struct PACKET_RECV
{
	int	type;
	int	seq_id;
	int	key_len;
};

/* 4, 5, 6, 7, 8 
 */
struct PACKET_HEAD_0
{
	int	type;
	int	seq_id; 
};

/* 0, 1, 3
 */
struct PACKET_HEAD_1
{
	int 	type;
	int 	seq_id;
	int	item_1_len;
};

/* 2
 */
struct PACKET_HEAD_2
{
	int 	type;
	int 	seq_id;
	int	item_1_len;
	int	item_2_len;
};

