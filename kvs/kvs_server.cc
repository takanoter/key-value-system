#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "src/server/server.h"
#include "src/server/worker.h"
#include "src/engine/engine.h"

kvs::HashEngine db;
int kvs_work(int fd, char* buf, int buf_size)
{
    kvs::Status s;
    kvs::Slice key,value;
    char buffer[1024];
    buf = buf;
    buf_size = buf_size;
    printf ("here do application kvs job with fd:%d.\n", fd);
    memset(buffer, 0, sizeof(buffer));
    read(fd, buffer, sizeof(buffer));
    printf ("receive buffer:[%s]\n", buffer);

    kvs::PutOptions put_opt;
    long long_keys = 1806;
    key.Set((const char*)&long_keys, sizeof(long));
    value.Set(buffer, strlen(buffer)+1);
    s = db.Put(put_opt, key, value);
    if (!s.ok()) { printf ("oh put failed.\n"); return 0; }

    printf ("now ok.\n");
    return 0;
}

int main()
{
    int ret = 0;
    char worker_name[1024];
    strcpy(worker_name, "xiaoming"); /*FIXME:need memset 0?*/
    kvs::SERVER server;
    server.Init(100, 100, 18086);  
    
    kvs::WORKER worker;
    worker.Init(100, kvs_work, worker_name, 100, NULL);
    server.AddWorker(&worker);

    kvs::EngineOptions engine_opt;
    kvs::Status s = db.Create(engine_opt, "mydb");
    if (!s.ok()) {
        printf ("Create failed.\n");
        return -1;
    }
    printf ("create db success.\n");

    server.Run();
    printf ("why not block?\n");
    return 0;
}
