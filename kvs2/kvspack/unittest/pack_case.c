#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../kvspack.h"

/*
typedef struct KVS_PACK {
    int *send_length;
    int buf_size;
    char *buf; 
} KVS_PACK;
*/
static void show(char* value, int value_len);
int main()
{
    int ret = 0;
    KVS_PACK* pack = NULL;   
    KVS_PACK* another_pack = NULL;
    char buf[1024];
    char *p;
    int value_len;
    int value_type;

    pack = kvsp_create(buf, sizeof(buf));
    ret = kvsp_put(pack, "key0", sizeof("key0"), 1, "value0", sizeof("value0"));
    ret = kvsp_put(pack, "key1", sizeof("key1"), 1, "value1", sizeof("value1"));
    kvsp_check(pack);
    
    kvsp_get(pack, "key1", sizeof("key1"), &value_type, (void**)&p, &value_len);
    show(p, value_len);
    
    another_pack = kvsp_load(buf, sizeof(buf));
    kvsp_check(another_pack);
    kvsp_get(another_pack, "key0", sizeof("key0"), &value_type, (void**)&p, &value_len);
    show(p, value_len);

    kvsp_destroy(pack);
    kvsp_destroy(another_pack);


    return 0;
}

static void show(char* value, int value_len)
{
    int i;
    printf ("show [%d]:", value_len);
    for (i=0; i<value_len; i++)
    {
        printf ("[%c]", value[i]);
    }
    printf ("\n\n");
}

