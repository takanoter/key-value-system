#include <stdio.h>
#include <string>
//#include "../configure.h"
#include "../kvs_slice.h"
//#include "../kvs_engine.h"
#include "../kvs_options.h"
#include "../engine.h"
//#include "unit_func.h"
using namespace kvs;
int main()
{
    HashEngine db;
    EngineOptions engine_opt;
    Status s = db.Create(engine_opt, "mydb");
    if (!s.ok()) {
        printf ("Create failed.\n");
        return -1;
    }
    return 0;
}

