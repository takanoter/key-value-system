#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../kvspack.h"

using namespace kvs;
static void show(char* value, int value_len);
int main()
{
    int ret = 0;
    char buf[1024], k[1024], v[1024];
    char *p;
    int value_len;
    int value_type;
    PACK pack, another_pack;

    pack.Init(buf, sizeof(buf));
    strcpy(k, "key0");
    strcpy(v, "value0");
    ret = pack.Put(k, sizeof("key0"), 1, v, sizeof("value0"));
    strcpy(k, "key1");
    strcpy(v, "value1");
    ret = pack.Put(k, sizeof("key1"), 1, v, sizeof("value1"));
    pack.Check();
    
    strcpy(k, "key1");
    pack.Get(k, sizeof("key1"), &value_type, (char**)&p, &value_len);
    show(p, value_len);
    
//    another_pack.Init(buf, sizeof(buf));
    another_pack.Load(buf, sizeof(buf));
    another_pack.Check();
    strcpy(k, "key0");
    another_pack.Get(k, sizeof("key0"), &value_type, (char**)&p, &value_len);
    show(p, value_len);

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

