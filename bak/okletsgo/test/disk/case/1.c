#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disk.h"
#include "mem_pool.h"

/*
case 1 PLAN:
1).Generate 100 {___,length};
2).disk_alloc, fill in {ptr, length};
3).disk_free i%2==0, in {ptr, length};
4).i%2==1:{ptr,length} --> bitmap
5).Check 20:
   Generate {___,length};
   disk_alloc, fill in {ptr, length};
   check if invvalid in [bitmap].
   fill in [bitmap].
*/

typedef struct PAIR
{
	int ptr;
	int length;
}PAIR;

static int get_random();
static void bitmap_fill(const int ptr, const int length);
static int bitmap_is_valid(const int ptr, const int length);

char bitmap[32*1024];
#define TEST_CASE_CNT_TOTAL 100
#define TEST_CASE_CNT_CHECK 100

int main()
{
	char* g_image;
	PAIR pairs[100];
	PAIR cur_pair;
	int i;


	mp_init();
	g_image=(char*)mp_alloc(IMAGE_K_SIZE*1024);

	/******************************************************************/

	disk_init(INIT_TYPE_CREATE, g_image+IMAGE_K_DISK*1024, "/tmp/log", 32*1024);
	for (i=0; i<32*1024; i++)
	{
		bitmap[i]=0;
	}
	
	/* PHASE 1,2 */
	for (i=0; i<TEST_CASE_CNT_TOTAL; i++)
	{
		pairs[i].length=get_random();
		pairs[i].ptr=disk_alloc(pairs[i].length);
	}
	printf ("phase 1,2 done.\n\n");

	/* PHASE 3 :free i%2==0 */
	for (i=0; i<TEST_CASE_CNT_TOTAL; i++)
	{
		if (i%2==1) continue;
		disk_free(pairs[i].ptr);
	}
	printf ("phase 3 done.\n\n");

	/* PHASE 4 :fill i%2==1 */	
	for (i=0; i<TEST_CASE_CNT_TOTAL; i++)
	{
		if (i%2==0) continue;
		bitmap_fill(pairs[i].ptr, pairs[i].length);
		printf ("fill bitmap %d done.\n",i);
	}
	printf ("phase 4 done.\n\n");

	/* PHASE 5 */
	for (i=0; i<TEST_CASE_CNT_CHECK; i++)
	{
		cur_pair.length=get_random();
		cur_pair.ptr=disk_alloc(cur_pair.length);
		printf ("Case %d: [ptr:%d],[length:%d], ----%d\n", i, cur_pair.ptr, cur_pair.length, bitmap_is_valid(cur_pair.ptr, cur_pair.length));
		bitmap_fill(cur_pair.ptr, cur_pair.length);
	}
	printf ("phase 5 done.\n\n");
	
	disk_exit();

	/******************************************************************/

	mp_free(g_image);
	mp_exit();

	return 0;
}



static int get_random()
{
#define RANDOM_MOD 320
	return rand()%RANDOM_MOD+1;
}

static int bitmap_is_valid(const int ptr, const int length)
{
	int i;
	for (i=ptr; i<ptr+length; i++)
	{
		if (bitmap[i]==1) return 0;
	}
	return 1;
}
	
static void bitmap_fill(const int ptr, const int length)
{
	int i;
	for (i=ptr; i<ptr+length; i++)
	{
		bitmap[i]=1;
	}
}
