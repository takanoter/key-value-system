#include <stdio.h>
#include <string>
#include "../configure.h"
#include "../kvs_slice.h"
#include "../kvs_status.h"
#include "../index.h"
#include "unit_func.h"
using namespace kvs;
int create();
int load();

int main()
{
    if (create()!=0) {
        printf ("test 1 failed.\n");
        return -1;
    }
    printf ("create test success.\n");

    if (load()!=0) {
        printf ("test 2 failed.\n");
        return -1;
    }

    printf ("hello!\n");
    return 0;
}

int create()
{
    INDEX index;
    CONFIGURE meta_conf;
    Status s = meta_conf.Create("data/index.dat");
    if (!s.ok()) return -1;

    s = FillMetaConfigure(meta_conf);
    meta_conf.Solid();
    
    int index_head_size = 1024; 
    s = index.Born(meta_conf, index_head_size);
    if (!s.ok()) return -1;

    Slice key;
    Offset off=0, len=0;
    long long_keys[7] = {1,1806,9,9595,5959,163,182};
    for (int i=0; i<7; i++) {
        key.Set((const char*)&long_keys[i], sizeof(long));
        s = index.Insert(true, key, long_keys[i]%10, long_keys[i]%10);
    }
    
    for (int i=0; i<7; i++) {
        key.Set((const char*)&long_keys[i], sizeof(long));
        s = index.Search(key, &off, &len);
        printf ("%ld-%lld-%lld\n", *((long*)key.data()), off, len);
    }

    s = UpdateMetaConfigure(meta_conf, index.GetIndexHorizon(), index.GetIndexFreeSlotHorizon());
    if (!s.ok()) {
        printf ("update meta configure failed.\n");
        return -1;
    }
    s = meta_conf.Solid();
    if (!s.ok()) {
        printf ("Solid failed.\n");
        return -1;
    }
    return 0;
}

int load() 
{
    INDEX index;
    CONFIGURE meta_conf;
    Status s = meta_conf.Load("data/index.dat");
    if (!s.ok()) return -1;
    printf ("here1\n");

    int index_head_size = 20;
    s = index.Load(meta_conf, index_head_size);
    if (!s.ok()) return -1;
    printf ("here2\n");


    Slice key;
    Offset off=0, len=0;
    long long_keys[7] = {1,1806,9,9595,5959,163,182};
    for (int i=0; i<7; i++) {
        key.Set((const char*)&long_keys[i], sizeof(long));
        s = index.Search(key, &off, &len);
        printf ("%ld-%lld-%lld\n", *((long*)key.data()), off, len);
    }

    return 0;
}
