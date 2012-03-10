#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct KV_SERVER {
    int backlog;
    int epool_queue_size;
    struct epoll_event *events;

    JOB jobs;
    int job_queue_size;

    int fresh_msec;

    int worker_thread_num;
    WORKER_FUNC worker_thread_func;
    pthread_t *worker_thread_id;
    
    int listen_port;
} KV_SERVER;

int server_run(KV_SERVER* server);

int server_set_queue_size(KV_SERVER* server, const int backlog, const int epool_queue_size, const int job_queue_size);

int server_set_thread(KV_SERVER* server, const int worker_thread_num, WORKER_FUNC worker_thread_func);

int server_set_timeout(KV_SERVER* server, const int fresh_msec);

int server_set_port(KV_SERVER* server, const int listen_port);

