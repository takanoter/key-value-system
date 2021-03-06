#ifndef KV_WORKER_H
#define KV_WORKER_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jobs.h"

namespace kvs {

typedef int(*WORKER_FUNC)(int fd, char* buf, int buf_size);
const int WORKER_NAME_SIZE = 64;

class WORKER
{
  public:
    WORKER() {};
    ~WORKER() {/*FIXME*/};
    bool Init(int job_size, WORKER_FUNC worker_thread_func, 
              char* worker_name, long long timeout, char* base_info);

    pthread_t* GetThreadIdP() {
        return &thread_id_;
    }

    bool FitName(const char* name) {
        if (strcmp(name, name_)==0) return true;
        return false;
    }

    //attention:good
    int Run(JOB_INFO& job_info) {
        return job_callback_(job_info.fd, job_info.buf, job_info.buf_size);
    }

  public:
    JOBS jobs_;

  private:
    WORKER_FUNC job_callback_; //read JOB_INFO, and do the job actually 
    char name_[WORKER_NAME_SIZE];
    pthread_t thread_id_;
    char* base_info_; //like HashEngine* etc. init in init
}; //class WORKER

void *worker_loop(void *arg);

}; //namespace kvs

#endif
