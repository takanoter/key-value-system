#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "worker.h"
#include "jobs.h"

void *worker(void *arg)
{
    int fd = -1;
    int ret = 0;
    JOBS* jobs = NULL;
    WORKER_INFO worker_info;
    WORKER_FUNC worker_thread_func = NULL;

    worker_info = *((WORKER_INFO*)arg);
    jobs = worker_info.jobs;
    worker_thread_func = worker_info.worker_thread_func;
    while (1) {
        fd = jobs_fetch(jobs);
        if (-1 == fd) {
            sleep(1);
        }
        ret = worker_thread_func(fd);
    }
}



