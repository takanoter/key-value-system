#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "server.h"
#include "worker.h"

int kvs_work(int fd, char* buf, int buf_size)
{
    char buffer[1024];
    buf = buf;
    buf_size = buf_size;
    printf ("here do application kvs job with fd:%d.\n", fd);
    memset(buffer, 0, sizeof(buffer));
    read(fd, buffer, sizeof(buffer));
    printf ("receive buffer:[%s]\n", buffer);
    printf ("now ok.\n");
    return 0;
}

int main()
{
    int ret = 0;
    char worker_name[1024];
    strcpy(worker_name, "xiaoming"); /*FIXME:need memset 0?*/
    kvs::SERVER server;
    server.Init(100, 100, 8086);  
    
    kvs::WORKER worker;
    worker.Init(100, kvs_work, worker_name, 200, NULL);
    server.AddWorker(&worker);
    server.Run();
    printf ("why not block?\n");
    return 0;
}
