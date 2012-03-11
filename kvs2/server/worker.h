#ifndef KV_WORKER_H
#define KV_WORKER_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jobs.h"
typedef int(*WORKER_FUNC)(int fd) ;
//typedef int(*func)(int fd) WORKER_FUNC;  why can't this?

typedef struct WORKER_INFO
{
    JOBS* jobs;
    WORKER_FUNC worker_thread_func;
}WORKER_INFO;


void *worker(void *arg);

#endif
