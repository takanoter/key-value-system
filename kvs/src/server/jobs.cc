#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <list>
#include "jobs.h"

namespace kvs {

bool JOBS::init(const int job_queue_size, long long timeout) {
    JOB job;
    job.used = false;

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
    pthread_mutex_lock(lock_);
    JOB_LIST::iterator it = jobs_.begin();
again: //attention: no need to do: it = jobs_.begin()
    for ( ; it!=jobs_.end(); ++it) {
        if (true == it->used) break;
    }
    if (it == jobs_.end()) {
        job_info.fd = -1;
        pthread_mutex_unlock(lock_);
        return job_info;
    }

    if ((long long)time(NULL) > it->start_time_point + timeout_) {
        printf("job timeout: fd:%d start_tiem[%lld]+wait_in_q_time[%lld] > cur_time[%lld]",
               it->job_info.fd, it->start_time_point, timeout_, (long long)time(NULL));
        close(it->job_info.fd);
        it->used = false;
        it->job_info.fd = -1;
        goto again;
    }

    job_info = it->job_info;
    it->used = false;
    pthread_mutex_unlock(lock_);
    return job_info;
}

bool JOBS::inject(int fd, char* buf, int buf_size) {
    JOB_INFO job_info;
    job_info.fd = fd;
    job_info.buf = buf;
    job_info.buf_size = buf_size;

    pthread_mutex_lock(lock_);
    JOB_LIST::iterator it = jobs_.begin();
    for ( ; it!=jobs_.end(); ++it) {
        if (false == it->used) break;
    }
    if (it == jobs_.end()) {
        pthread_mutex_unlock(lock_);
        return false;
    }
    it->job_info = job_info;
    it->used = true;
    it->start_time_point = time(NULL);
    
    pthread_mutex_unlock(lock_);
    return true;
}

}; //namespace kvs

