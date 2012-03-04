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

#define	VALUE_BUFFER_SIZE (5*1024*1024)
#define	KEY_BUFFER_SIZE (1024)
char value_buffer[VALUE_BUFFER_SIZE];
char key_buffer[KEY_BUFFER_SIZE];

int main(int argc, char** argv)
{
	KVS_SET kvs;
	int cur_time, cnt, cur_id;
	time_t now;
	struct tm* timenow;
	
	if (argc!=4)
	{
		printf ("Usage:\n");
		printf ("~/kvs/test$ ./lib_get BigfilePathname IndexFile Count\n");
		printf ("an example: ./lib_get /tmp/kvs.disk ./index_random 5000\n");
		printf ("System will LOAD a kvs instance(big file) from '/tmp/kvs.disk', read '5000' iterms from 'index_random' file as keys.\n");
		printf ("\n");
		return 0;
	}

	char bigfile[1024];
	char indexfile[1024];
	memset(bigfile, 0, sizeof(bigfile));
	memset(indexfile, 0, sizeof(indexfile));

	sscanf(argv[1], "%s", bigfile);
	sscanf(argv[2], "%s", indexfile);
	sscanf(argv[3], "%d", &cnt);
	printf ("bigfile:%s\tindex file:%s\tcount:%d\n", bigfile, indexfile, cnt);

	FILE* fp=fopen(indexfile, "r");


/* KVSystem init */
	kvs.init_type=KVS_LOAD;
	kvs.size_in_g=500;
	strcpy(kvs.disk_file, bigfile);
	strcpy(kvs.disk_log, "/tmp/kvs_get_disk.log");
	strcpy(kvs.sync_log, "/tmp/kvs_get_sync.log");
	strcpy(kvs.index_log, "/tmp/kvs_get_index.log");
	
	kv_init(&kvs,NULL);
	
	time(&now);
	timenow=localtime(&now);
	cur_time=timenow->tm_sec+(timenow->tm_min)*60+(timenow->tm_hour)*60*60;

	for (cur_id=0; cur_id<cnt; cur_id++)
	{
		memset(key_buffer, 0, sizeof(key_buffer));
		fscanf (fp, "%s", key_buffer);
		kv_get(key_buffer, 32 , value_buffer, VALUE_BUFFER_SIZE);
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


