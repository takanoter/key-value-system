#ifndef KV_JOBS_H
#define KV_JOBS_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>

#include <pthread.h>

namespace kvs {
typedef struct JOB_INFO
{
    int fd;
    char *buf;
    int buf_size;
    JOB_INFO() {
        fd = -1;
        buf = NULL;
        buf_size = 0;
    }
}JOB_INFO;

typedef struct JOB
{
    bool is_used;
    JOB_INFO job_info;
    long long start_time_point;
//inline void Status::operator=(const Status& s) {

    void operator = (JOB const& job) {
        is_used = job.is_used;
        job_info = job.job_info;
        start_time_point = job.start_time_point;
    }
    JOB() {};
}JOB;

typedef std::list<JOB> JOB_LIST;
class JOBS
{
  public:
    JOBS(){};
    ~JOBS(){
        jobs_.clear();
        pthread_mutex_destroy(lock_);
    };    

    JOB_INFO fetch();
    bool inject(int fd, char* buf, int buf_size);

    bool init(const int job_queue_size, long long timeout) ;

  private:
    JOB_LIST jobs_;
    long long timeout_;
    pthread_mutex_t *lock_;
}; //class JOBS;

}; //namespace kvs
#endif
