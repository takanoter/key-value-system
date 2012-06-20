#ifndef KV_SERVER_H
#define KV_SERVER_H 1

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

#include "jobs.h"
#include "worker.h"

namespace kvs {

class SERVER {
  public:
    SERVER() {};
    ~SERVER() {};
    int Run();
    bool AddWorker(WORKER* worker);
    bool Init(int backlog, int epoll_queue_size, int listen_port);

  private:
    int backlog_;
    int listen_port_;
    int epoll_queue_size_;
    struct epoll_event *events_;
    
    std::list <WORKER*> workers_; 
  
  private:
    int deal_accept(int listen_fd);
    int deal_listen(int port, int backlog);
    void setnonblocking(int fd);
    void deal_with_fd(int conn_sock);

}; //class SERVER

}; //namespace kvs
#endif

