#include <stdio.h>
#include <stdlib.h>
#include "kvs_utils.h"

unsigned long long _kv_consequence_id()
{
    static unsigned long long i = 0;
    return ++i;
}

unsigned long long _get_hashhead_id(unsigned long long key_sign, unsigned long long length)
{
    unsigned long long ret = key_sign % length;
    return ret;
}


unsigned long long create_key_sign(const char* str, int key_len)
{
    unsigned hash = 0;
    int i;
 
	for (i=0; i < key_len; i++) {
		if ((i & 1) == 0) {
			hash ^= ((hash << 7) ^ (*(str+i)) ^ (hash >> 3));
		} else {
			hash ^= (~((hash << 11) ^ (*(str+i)) ^ (hash >> 5)));
		}
	}
	return hash;
}
