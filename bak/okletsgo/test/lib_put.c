#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include <kvs.h>

static void get_random(char *p, const int size);

#define	VALUE_BUFFER_SIZE (100*1024)
#define	KEY_BUFFER_SIZE (32)
char value_buffer[VALUE_BUFFER_SIZE];
char key_buffer[KEY_BUFFER_SIZE];

int main(int argc, char** argv)
{
	KVS_SET kvs;
	int cur_time, cnt, i, cur_id;
	time_t now;
	struct tm* timenow;

	if (argc!=4)
	{
		printf ("Usage:\n");
		printf ("~/kvs/test$ ./lib_put BigfilePathname OutputIndexFile Count\n");
		printf ("an example: ./lib_put /tmp/kvs.disk ./index 10000\n");
		printf ("System will CREATE a kvs instance(big file) at '/tmp/kvs.disk', generate (Key, Value) pair and put into system, value size is 100K, KEYs are written into './index' file.\n");
		printf ("\n");
		return 0;
	}
	char bigfile[1024];
	char indexfile[1025];
	memset(bigfile, 0, sizeof(bigfile));
	memset(indexfile, 0, sizeof(indexfile));

	sscanf(argv[1], "%s", bigfile);
	sscanf(argv[2], "%s", indexfile);
	sscanf(argv[3], "%d", &cnt);
	
	FILE* fp=fopen(indexfile, "w");

	for (i=0; i<VALUE_BUFFER_SIZE; i++)
	{
		value_buffer[i]='a';
	}
	

/* KVSystem init */
	kvs.init_type=KVS_CREATE;
	kvs.size_in_g=500;
	strcpy(kvs.disk_file, bigfile);
	strcpy(kvs.disk_log, "/tmp/kvs_put_disk.log");
	strcpy(kvs.sync_log, "/tmp/kvs_put_sync.log");
	strcpy(kvs.index_log, "/tmp/kvs_put_index.log");
	
	kv_init(&kvs,NULL);
	
	time(&now);
	timenow=localtime(&now);
	cur_time=timenow->tm_sec+(timenow->tm_min)*60+(timenow->tm_hour)*60*60;

	for (cur_id=0; cur_id<cnt; cur_id++)
	{
		memset(key_buffer, 0, sizeof(key_buffer));
		get_random(key_buffer, KEY_BUFFER_SIZE);
		fprintf (fp, "%s\n", key_buffer);
		kv_put(key_buffer, KEY_BUFFER_SIZE, value_buffer, VALUE_BUFFER_SIZE);
		if (cur_id%500==0)
		{
			time(&now);
			timenow=localtime(&now);
			cur_time=timenow->tm_sec+(timenow->tm_min)*60+(timenow->tm_hour)*60*60;
			printf ("%d: sec:%d\n",cur_id, cur_time);
			fflush(stdout);
		}
	}

	kv_exit();
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

