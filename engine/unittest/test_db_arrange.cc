#include <stdio.h>
#include <string>
#include "../kvs_slice.h"
#include "../kvs_options.h"
#include "../engine.h"
using namespace kvs;
void get(HashEngine& db);
int main()
{
    HashEngine db;
    EngineOptions engine_opt;
    Status s = db.Open(engine_opt);
    if (!s.ok()) {
        printf ("Create failed.\n");
        return -1;
    }
    printf ("open success.\n");
    get(db);
    printf ("mission complete.\n");
    return 0;
}

void get(HashEngine& db) {
    Status s;
    Slice key, value;
    long long_keys[7] = {1,1806,9,9595,5959,163,182};
    char buffer[1024];

    GetOptions get_opt;
    for (int i=0; i<7; i++) {
        std::string value;
        key.Set((const char*)&long_keys[i], sizeof(long));
        s = db.Get(get_opt, key, &value);
        if (!s.ok()) { printf ("oh get failed.\n"); /*return; */}
        printf ("[%d] %ld:%s\n", i, long_keys[i], value.c_str());
    }

    ArrangeOptions arrange_opt;
    s = db.Arrange(arrange_opt);
    if (!s.ok()) {
        printf ("arrange failed.\n");
    }
    return;
}

