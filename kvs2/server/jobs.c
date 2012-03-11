#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

typedef struct JOB
{
    int is_used;
    int fd;
    int timeout;
}JOB;

typedef struct JOBS
{
    int size;
    JOB* jobs;
    int timeout;
    pthread_mutex_t *lock;
}JOBS;

int jobs_fetch(JOBS* jobs)
{
    int i = 0;
    int ret_fd = -1;
    {
        pthread_mutex_lock(jobs->lock);
        for (i=0; i<jobs->size; i++) {
            if (1 == jobs->jobs[i].is_used) {
                ret_fd = jobs->jobs[i].fd;
                jobs->jobs[i].is_used = 0;
                jobs->jobs[i].fd = -1;
                break;
            }
        }
        pthread_mutex_unlock(jobs->lock);
    }
    return ret_fd;
}

int jobs_push(JOBS* jobs, int fd)
{
    int i = 0;
    int ret = 0;
    {
        pthread_mutex_lock(jobs->lock); /*TODO:bad lock, need while(1)?*/
        for (i=0; i<jobs->size; i++) { /*TODO:alrithm*/
            if (0 == jobs->jobs[i].is_used ) {
                break;
            }
        }
        if (i == jobs->size) {
            printf ("jobs queue full[%d].\n", jobs->size);
            ret = -1;
        } else {
            jobs->jobs[i].is_used = 1;
            jobs->jobs[i].fd = fd;
        }
        pthread_mutex_unlock(jobs->lock);
    }
    return ret;
}

int jobs_init(JOBS *jobs, const int job_queue_size)
{
    int ret = 0;
    int i = 0;
    if (NULL == jobs) {
        ret = -1;
        goto OUT;
    }

    jobs->size = job_queue_size;

    jobs->jobs = malloc(sizeof(JOB) * job_queue_size);
    if (NULL == jobs->jobs) {
        ret = -1;
        goto OUT;
    }
    for (i=0; i<job_queue_size; i++) {
        jobs->jobs[i].is_used = 0;
        jobs->jobs[i].fd = -1;
    }

    jobs->lock = malloc(sizeof(pthread_mutex_t));
    if (NULL == jobs->lock) {
        ret = -1;
        goto OUT;
    }
    pthread_mutex_init(jobs->lock, NULL);

OUT:
    if (NULL != jobs->jobs) {
        free(jobs->jobs);
    }
    if (NULL != jobs->lock) {
        pthread_mutex_destroy(jobs->lock);
    }
    
    return ret;
}

int jobs_destroy(JOBS *jobs)
{
    /*TODO*/
    return 0;
}
