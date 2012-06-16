#include <stdio.h>
#include <string>
#include "../kvs_slice.h"
#include "../kvs_options.h"
#include "../engine.h"
using namespace kvs;
void load(HashEngine & db);
int main()
{
    HashEngine db;
    EngineOptions engine_opt;
    Status s = db.Create(engine_opt, "mydb");
    if (!s.ok()) {
        printf ("Create failed.\n");
        return -1;
    }
    printf ("create db success.\n");

    load(db);
    printf ("load success.\n");
    s = db.Solid();
    if (!s.ok()) {
        printf ("Solid failed.\n");
        return -1;
    }
    printf ("mission complete.\n");

    return 0;
}

void load(HashEngine & db) {
    Status s;
    Slice key, value;
    long long_keys[7] = {1,1806,9,9595,5959,163,182};
    char buffer[1024];

    PutOptions put_opt;
    for (int i=0; i<7; i++) {
        key.Set((const char*)&long_keys[i], sizeof(long));
        memset(buffer,0, sizeof(buffer));
        sprintf(buffer, "%ld", long_keys[i]*2);
        value.Set(buffer, strlen(buffer)+1);
        s = db.Put(put_opt, key, value);
        if (!s.ok()) { printf ("oh put failed.\n"); return; }
    }

    GetOptions get_opt;
    for (int i=0; i<7; i++) {
        std::string value;
        key.Set((const char*)&long_keys[i], sizeof(long));
        s = db.Get(get_opt, key, &value);
        if (!s.ok()) { printf ("oh get failed.\n"); return; }
        printf ("[%d] %ld:%s\n", i, long_keys[i], value.c_str());
    }

    return ;
}
