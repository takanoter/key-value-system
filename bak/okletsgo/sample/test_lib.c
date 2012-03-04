#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kvs.h>
#define SAMPLE_CNT 5
#define BUFFER_SIZE (5*1024*1024)

char keys[SAMPLE_CNT][8]={"bb","aa","yy","xx", "bbbb"};
char values[SAMPLE_CNT][8]={"bbbb","aaaa","yyyy","xxxx","bbbbbbbb"};
char buffer[BUFFER_SIZE];

int main()
{
	char buf[1024];
	int i,op;
	struct KVS_ENV kvs;
	kvs.init_type=KVS_CREATE;  // or KVS_LOAD if load kvs.
	kvs.size_in_g=1000;
	kvs.disk_file="/tmp/sample_disk";
	kvs.disk_log="/tmp/kvs_disk.log";
	kvs.sync_log="/tmp/kvs_sync.log";
	kvs.index_log="/tmp/kvs_index.log";

	kv_init(&kvs);

	for (i=0; i<SAMPLE_CNT; i++) {
		kv_put(keys[i], strlen(keys[i]), values[i], strlen(values[i]) );
	}

	for (i=0; i<SAMPLE_CNT; i++) {
		memset(buffer, 0, sizeof(buffer));
		kv_get(keys[i], strlen(keys[i]), buffer, BUFFER_SIZE);
		printf ("%s--%s\n", keys[i], buffer);
	}

	for (i=0; i<SAMPLE_CNT; i++) {
		kv_delete( keys[i], strlen(keys[i]) );
	}

	kv_exit();
	return 0;
}
