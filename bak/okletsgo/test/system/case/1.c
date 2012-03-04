#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kvs.h"

#define	SAMPLE_CNT	5
char* keys[]={"he","hi","hg","hh","gg"};
char* values[]={"hehe", "hihi", "hghg", "hhhh", "gggg"};

int main()
{
	KVS_SET kvs;
	int i;
	char buf[1024];

	kvs.init_type=KVS_CREATE;  // or KVS_LOAD if load kvs.
	kvs.size_in_g=1000;
	strcpy(kvs.disk_file, "/tmp/disk");
	strcpy(kvs.log, "/tmp/log.kvs");
	strcpy(kvs.disk_log, "/tmp/log.disk");
	strcpy(kvs.sync_log, "/tmp/log.sync");
	strcpy(kvs.index_log, "/tmp/log.index");

	kv_init_2(&kvs);
	/************oooooooo******************************************************/

	for (i=0; i<SAMPLE_CNT; i++)
	{
		kv_put( keys[i], strlen(keys[i]), values[i], strlen(values[i]) );
	}

	for (i=0; i<SAMPLE_CNT; i++) 
	{
		kv_get( keys[i], strlen(keys[i]), buf, 1024 );
		printf ("GET:%s  \t:\t%s\n",keys[i], buf);
	}

	for (i=0; i<SAMPLE_CNT; i++)
	{
		kv_delete( keys[i], strlen(keys[i]));
	}

	/******ooooooo******o******************************************************/

	kv_exit();

	return 0;
}
