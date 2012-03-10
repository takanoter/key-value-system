#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "server.h"
#include "worker.h"

int kvs_work(int fd)
{
    printf ("here do application kvs job with fd:%d.\n", fd);
    return 0;
}

int main()
{
    int ret = 0;
    KV_SERVER server;
    ret = server_set_queue_size(&server, 100, 100, 100);
    ret = server_set_thread(&server, 2, kvs_work);
    ret = server_set_timeout(&server, 1000);
    ret = server_set_port(&server, 38000);
    ret = server_fun(&server);
    printf ("why not block?\n");
    return 0;
}
