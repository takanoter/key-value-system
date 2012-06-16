#include <stdio.h>
#include <string>
#include "../configure.h"
#include "../kvs_slice.h"
#include "unit_func.h"
using namespace kvs;
int main()
{
    CONFIGURE conf;
    conf.Create("data/create_data_conf.dat");
    Status s = FillDataConfigure(conf);
    if (!s.ok()) {
        printf ("FillConfigure failed.\n");
        return -1;
    }

    s = conf.Solid();
    if (!s.ok()) {
        printf ("Solid failed.\n");
        return -1;
    }

    CONFIGURE new_conf;
    s = new_conf.Load("data/create_data_conf.dat");
    if (!s.ok()) {
        printf ("Load failed.\n");
        return -1;
    }
    s = LoadDataConfigure(new_conf);
    if (!s.ok()) {
        printf ("LoadDataConfigure failed.\n");
        return -1;
    }

    return 0;
}

