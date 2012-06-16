#include <stdio.h>
#include <string>
#include "../configure.h"
#include "../kvs_slice.h"
#include "../kvs_status.h"
#include "../space.h"
#include "unit_func.h"
using namespace kvs;
int create();
int load();

int main()
{
    int ret = create();
    if (ret!=0) {
        printf ("test 1 failed with ret:%d.\n", ret);
        return -1;
    }
    printf ("create test success.\n");

    if (load()!=0) {
        printf ("test 2 failed.\n");
        return -1;
    }

    printf ("mission complete!\n");
    return 0;
}

int create()
{
    SPACE space;
    CONFIGURE data_conf;
    Status s = data_conf.Create("data/data.dat");
    if (!s.ok()) return -1;

    s = FillDataConfigure(data_conf);
    if (!s.ok()) return -2;
    s = data_conf.Solid();
    if (!s.ok()) return -3;

    s = space.Born(data_conf);
    if (!s.ok()) return -4;

    Slice key, value;
    Offset off=0, len=0;
    long long_keys[7] = {1,1806,9,9595,5959,163,182};
    char buffer[1024];
    Offset last_offset = 0;
    data_conf.FetchLastOffset(&last_offset);
    for (int i=0; i<7; i++) {
        key.Set((const char*)&long_keys[i], sizeof(long));
        memset(buffer,0, sizeof(buffer));
        sprintf(buffer, "%ld", long_keys[i]*2);
        value.Set(buffer, strlen(buffer)+1);
        s = space.Write(last_offset, i, key, value, false);
        printf ("last_offset %lld: %ld-%s length:%lld\n", last_offset, long_keys[i], value.data(),
                                                          space.CalLength(key,value));
        last_offset += space.CalLength(key, value);
    }
    
    s = data_conf.Solid();
    if (!s.ok()) {
        printf ("Solid failed.\n");
        return -1;
    }
    return 0;
}

int load() 
{
    SPACE space;
    CONFIGURE data_conf;
    printf ("here0\n");
    Status s = data_conf.Load("data/data.dat");
    if (!s.ok()) return -1;
    printf ("here1\n");

    s = space.Load(data_conf);
    if (!s.ok()) return -1;
    printf ("here2\n");

    std::string value;
    long long_keys[7] = {1,1806,9,9595,5959,163,182};
    long kv_offset[7] = {160, 186, 215, 242, 272, 302, 330};
    long kv_length[7] = {26, 29, 27, 30, 30, 28, 28};

    for (int i=0; i<7; i++) {
        s = space.Read(kv_offset[i], kv_length[i], &value);
        printf ("offset %ld: %s\n", kv_offset[i], value.c_str());
    }

    return 0;
}
