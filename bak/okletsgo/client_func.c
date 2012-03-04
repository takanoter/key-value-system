#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <unistd.h>

#include "packet.h"
#include "kvsclient.h"
/*
use writev, readv ?
effect on writev+TCP: reliable???
*/
	
/* return data-size to be read if necessary.
 */
int client_recv_head(int fd, struct KV_RECV_HEAD * kv_recv_head)
{
	assert(kv_recv_head);
	int n, ret;
	n = read(fd, kv_recv_head, sizeof(struct KV_RECV_HEAD));
	if (kv_recv_head->type == KVSP_ANS_GET_YES) {
		n = read(fd, &ret, sizeof(int));
		printf ("to be read:%d\n", ret);
		return ret;
	}
	return 0;
}

void client_recv_date(int fd, int size, char* buf, int buf_size)
{
	assert(buf_size >= size);
	int n;
	n = read(fd, buf, size); 	
	return;
}
	
/*FIXME: use writev() instead.
-->Must keep them in one system call() write(fd, ...).
 */
int client_send (int fd, struct KV_SEND_HEAD* kv_send_head)
{
	assert(kv_send_head);
	char* this_will_change = NULL;
	char* p = NULL;
	int packet_size;
	if (kv_send_head->type == KVSP_REQ_GET || kv_send_head->type == KVSP_REQ_DEL) {
		packet_size = sizeof(struct PACKET_HEAD_1) + kv_send_head->key_len;
		this_will_change = malloc(packet_size);

		((struct PACKET_HEAD_1*) this_will_change)->type = kv_send_head->type;
		((struct PACKET_HEAD_1*) this_will_change)->seq_id = kv_send_head->seq_id;
		((struct PACKET_HEAD_1*) this_will_change)->item_1_len = kv_send_head->key_len;
		p = this_will_change + sizeof(struct PACKET_HEAD_1);
		strncpy(p, kv_send_head->key, kv_send_head->key_len);
		
		write(fd, this_will_change, packet_size);
		free(this_will_change); 
	} else if (kv_send_head->type == KVSP_REQ_PUT) { 
		packet_size = sizeof(struct PACKET_HEAD_2) + kv_send_head->key_len + kv_send_head->value_len;
		this_will_change = malloc(packet_size);

		((struct PACKET_HEAD_2*) this_will_change)->type = kv_send_head->type;
		((struct PACKET_HEAD_2*) this_will_change)->seq_id = kv_send_head->seq_id;
		((struct PACKET_HEAD_2*) this_will_change)->item_1_len = kv_send_head->key_len;
		((struct PACKET_HEAD_2*) this_will_change)->item_2_len = kv_send_head->value_len;
		p = this_will_change + sizeof(struct PACKET_HEAD_2);
		strncpy(p, kv_send_head->key, kv_send_head->key_len);
		p += kv_send_head->key_len;          /* or p = strncpy() ?? */
		strncpy(p, kv_send_head->value, kv_send_head->value_len);
		
		write(fd, this_will_change, packet_size);
		free(this_will_change);
	} else {
		printf ("should not be here. kv_send_head->type error.\n");
	}

	return 0;
}

void client_close(int fd)
{
	struct PACKET_HEAD_0 packet_head;
	packet_head.type = KVSP_REQ_CLOSE;
	write(fd, &packet_head, sizeof(struct PACKET_HEAD_0));
	return;
}

