#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "worker.h"
//#include "jobs.h"

namespace kvs {

void *worker_loop(void *arg)
{
    int fd = -1;
    int ret = 0;

    WORKER* worker = ((WORKER*)arg);
    while (1) {
        JOB_INFO job_info = worker->jobs_.fetch();
        if (-1 == job_info.fd) {
            sleep(1); /*FIXME*/
            continue;
        }
        ret = worker->Run(job_info);
        close(job_info.fd);
        free(job_info.buf);
    }
}

bool WORKER::Init(int job_size, WORKER_FUNC worker_thread_func, 
                  char* worker_name, long long timeout,
                  char* base_info) 
{
    bool s;
    s = jobs_.init(job_size, timeout);
    if (!s) return s;

    job_callback_ = worker_thread_func; 

    memset(name_, 0, sizeof(name_));
    strcpy(name_, worker_name);

    return true;
}

}; //namespace kvs
