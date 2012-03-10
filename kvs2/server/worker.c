#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "worker.h"

typedef int(*func)(int fd) WORKER_FUNC;

typedef struct WORKER_INFO
{
    WORKER_FUNC worker_thread_func;
    KV_SERVER *server;
}WORKER_INFO;


void *worker(void *arg)
{
    int ret;
    WORKER_INFO worker_info;
    worker_info = *((WORKER_INFO*)arg);
    server = worker_info.server;
    worker_thread_func = worker_info.worker_thread_func;
    while (1) {
        fd = jobs_fetch(server->jobs);
        if (-1 == fd) {
            sleep(1);
        }
        ret = worker_thread_func(fd);
    }
}



