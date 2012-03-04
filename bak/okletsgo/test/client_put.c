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

static void get_random(char *p, const int size);

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
	printf ("count:%d ip:%s:%d   index_file:%s\n", cnt, ip, port, index_file);
	
	FILE* fp=fopen(index_file, "w");
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
		get_random(key_buffer, KEY_BUFFER_SIZE);
		fprintf (fp, "%s\n", key_buffer);

		//kv_put(key_buffer, KEY_BUFFER_SIZE, value_buffer, VALUE_BUFFER_SIZE);
		value_buffer_size=VALUE_BUFFER_SIZE;
                op=data_trans(PUT, sockfd, key_buffer, KEY_BUFFER_SIZE, &value_buffer, &value_buffer_size);
		if (!op)
		{
			//printf ("PUT SUCCESS.\n");
		}
		else
		{
			printf ("PUT FAILED.\n");
		}

		if (cur_id%500==0)
		{
			time(&now);
			timenow=localtime(&now);
			cur_time=timenow->tm_sec+(timenow->tm_min)*60+(timenow->tm_hour)*60*60;
			printf ("%d: sec:%d\n",cur_id, cur_time);
			fflush(stdout);
		}
	}

	fclose(fp);
	return 0;
}

static void get_random(char *p, const int size)
{
	int i;
	for (i=0; i<size; i++)
	{
		p[i]='a'+(random()%26);
	}
	p[size-1]=0;
}

/*
#define NPIDS 9

char data_file[NPIDS][16]={ "2k.dat", "5k.dat", "10k.dat", "50k.dat","100k.dat","200k.dat", "1m.dat", "4m.dat", "5m.dat"};
char data_dir[32]="/home/hovery/data/";
char dest_dir[16]="data/";

    pid_t pid, pids[NPIDS];
    char datapath[32], *value, *valget, key[8];
    FILE *fp;
    struct stat st;
    struct sockaddr_in servaddr;
    freopen("client.log", "a", stderr);
    int filesize, valget_len;
    for (i=0;i<NPIDS;i++)
    {
        if((pid=fork())>0)
            pids[i]=pid;
        else
        {
            int count=0;
            memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(8080);
            inet_pton(AF_INET, "127.0.0.1", &(servaddr.sin_addr));
            sockfd = conn_serv(&servaddr);  
            memset(datapath, 0, sizeof(datapath));
            strcpy(datapath, data_dir);
            strcat(datapath, data_file[i]);
            fp = fopen(datapath, "rb");
            stat(datapath, &st);
            filesize=st.st_size;
            value=(char*)malloc(filesize);
            if (!value)
            {
                printf("Malloc failed!\n");
                exit(-1);
            }
            
            memset(value, 0, filesize);
            n=fread(value,sizeof(char),filesize,fp);
            fprintf(stderr, "read %d %d from %s\n", n, filesize, datapath);
*/
