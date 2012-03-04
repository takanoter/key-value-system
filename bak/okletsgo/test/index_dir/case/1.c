/* 
 * index.c's test:
 * Small Delete
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "index.h"
#include <kvs.h>

static void get_random(char *p, const int size);

int main(int argc, char** argv)
{
	char *g_image;
	int cnt, i, ret, result;
	char key[32][32];

	g_image=(char*)malloc(IMAGE_K_SIZE*1024);
	if (g_image==NULL)
	{
		printf ("mp_alloc g_image failed.\n");
		return 1;
	}
	idx_init(KVS_CREATE, g_image, "/tmp/index.log");

	printf ("idx_init done.\n");
	cnt=10;
	for (i=0; i<cnt; i++)
	{
		get_random(key[i], 32);
		printf ("insert:%s\n", key[i]);
		ret=idx_insert(key[i], 25, i, i);
		if (ret!=IDX_SUCCESS) {
			printf ("case %d: failed. %s\n", i, key[i]);
		}
	}

	idx_show();

/*
	idx_delete(key[4], 25, &result);
	idx_show();
 */

	idx_delete(key[1], 25, &result);
	idx_show();

	idx_exit();
	free(g_image);
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
