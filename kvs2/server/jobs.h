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

int jobs_fetch(JOBS* jobs);

int jobs_push(JOBS* jobs, int fd);

int jobs_init(JOBS *jobs, const int job_quue_size);

int jobs_destroy(JOBS *jobs);
