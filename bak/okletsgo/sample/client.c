/* Be sure server is running. */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <kvsclient.h>

struct KV_SEND_HEAD kv_send_sample = {
	type:		KVSP_REQ_PUT,
	seq_id:		10086,
	key_len:	5,
	value_len:	8,
	key:		"hello",
	value:		"student.",
};

struct KV_RECV_HEAD kv_recv_sample;


int main(int argc, char** argv)
{
	int sockfd;
	struct sockaddr_in servaddr;

        memset(&servaddr, 0, sizeof(servaddr));
	
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(29866); /* Specify Server Listen Port. */
        inet_pton(AF_INET, "127.0.0.1", &(servaddr.sin_addr)); /*Specify Server IP */

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if ( connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
		printf ("connect error");
		return 0;
	}


	client_send(sockfd, &kv_send_sample);

	int ret=client_recv_head(sockfd, &kv_recv_sample);
	printf ("ret:%d type:%d seq_id:%d\n", ret, kv_recv_sample.type, kv_recv_sample.seq_id);

	client_close(sockfd);

	return 0;
}

