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

#include <kvsp.h>

#define	VALUE_BUFFER_SIZE (100*1024)
#define	KEY_BUFFER_SIZE (32)

int main(int argc, char** argv)
{
	int cur_time, cnt, i, cur_id, value_buffer_size, sockfd, op, port;
	time_t now;
	struct tm* timenow;
	struct sockaddr_in servaddr;
	char *value_buffer=malloc(VALUE_BUFFER_SIZE);
	char *key_buffer=malloc(KEY_BUFFER_SIZE);
	char ip[1024];
	char index_file[1024];
	memset(ip, 0, sizeof(ip));
	memset(index_file, 0, sizeof(index_file));

	if (argc!=5)
	{
		printf ("usage: count ip port index_file\n\n");
		return 0;
	}
	sscanf(argv[1], "%d", &cnt);
	sscanf(argv[2], "%s", ip);
	sscanf(argv[3], "%d", &port);
	sscanf(argv[4], "%s", index_file);
	printf ("get from server, count:%d  ip:%s:%d  from index_file:%s\n", cnt, ip, port, index_file);
	
	FILE* fp=fopen(index_file, "r");
	if (fp==NULL)
	{
		printf ("file open failed.\n");
		return 0;
	}

	for (i=0; i<VALUE_BUFFER_SIZE; i++)
	{
		value_buffer[i]='a';
	}
	

	time(&now);
	timenow=localtime(&now);
	cur_time=timenow->tm_sec+(timenow->tm_min)*60+(timenow->tm_hour)*60*60;

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &(servaddr.sin_addr));
        sockfd = conn_serv(&servaddr);  

	for (cur_id=0; cur_id<cnt; cur_id++)
	{
		memset(key_buffer, 0, sizeof(key_buffer));
		fscanf (fp, "%s", key_buffer);

		value_buffer_size=VALUE_BUFFER_SIZE;
                op=data_trans(GET, sockfd, key_buffer, KEY_BUFFER_SIZE, &value_buffer, &value_buffer_size);
		if (!op)
		{
			free(value_buffer);
			//printf ("GET SUCCESS.\n");
		}
		else
		{
			printf ("GET FAILED.\n");
		}

		if (cur_id%500==0)
		{
			time(&now);
			timenow=localtime(&now);
			cur_time=timenow->tm_sec+(timenow->tm_min)*60+(timenow->tm_hour)*60*60;
			printf ("PROCESS:%d\tCount:%d\tsec:%d\n",getpid(), cur_id, cur_time);
			fflush(stdout);
		}
	}

	fclose(fp);
	return 0;
}


