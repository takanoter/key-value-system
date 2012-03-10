#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int(*func)(int fd) WORKER_FUNC;

typedef struct WORKER_INFO
{
    WORKER_FUNC worker_thread_func;
    KV_SERVER *server;
}WORKER_INFO;


void *worker(void *arg);

