#include <stdio.h>
#include <stdlib.h>
#include "jobs.h"

namespace kvs {

bool JOBS::init(const int job_queue_size, long long timeout) {
    JOB job;
    job.is_used = false;

    for (int i=0; i<job_queue_size; i++) {
       jobs_.push_back(job);
    }

    timeout_ = timeout;
        
    lock_ = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if (NULL == lock_) return false;
    pthread_mutex_init(lock_, NULL);

    return true;
}

JOB_INFO JOBS::fetch()
{
    JOB_INFO job_info;
    JOB_LIST::iterator it = jobs_.begin();
    for ( ; it!=jobs_.end(); ++it) {
        if (true == it->is_used) break;
    }
    if (it == jobs_.end()) {
        job_info.fd = -1;
        return job_info;
    }

    job_info = it->job_info;
    it->is_used = false;
    return job_info;
}

bool JOBS::inject(int fd, char* buf, int buf_size) {
    JOB_INFO job_info;
    job_info.fd = fd;
    job_info.buf = buf;
    job_info.buf_size = buf_size;

    JOB_LIST::iterator it = jobs_.begin();
    for ( ; it!=jobs_.end(); ++it) {
        if (false == it->is_used) break;
    }
    if (it == jobs_.end()) return false;
    it->job_info = job_info;
    return true;
}

}; //namespace kvs

