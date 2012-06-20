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

#include "server.h"
#include "jobs.h"
#include "worker.h"

namespace kvs {

#define MAXEPOLLS 1024
int SERVER::Run()
{
    struct epoll_event ev;
    int i = 0;
    int nfds;
    int epoll_fd;
    int connect_fd;
    int listen_fd = 0;

    listen_fd = deal_listen(listen_port_, backlog_);
    if ((epoll_fd = epoll_create(MAXEPOLLS)) < 0) {
        perror("epoll_create error");
        exit(-1);
    }
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
        perror("epoll_ctl add error");
        exit(-1);
    }
    
    std::list <WORKER*> ::iterator it; 
    for (it = workers_.begin(); it!=workers_.end(); ++it) {
         pthread_create((*it)->GetThreadIdP(), NULL, worker_loop, *it);
    }

    while (1) {
        //printf ("listen_fd:%d, epoll_fd:%d\n", listen_fd, epoll_fd);
        nfds = epoll_wait(epoll_fd, events_, epoll_queue_size_, 50/*ms FIXME*/); 
        /*functions max sync time: compilable style */
        if (nfds == -1) {
            perror("epoll_wait error");
            exit(-1);
        }

        for (i = 0; i < nfds; i++) {
            if (events_[i].data.fd == listen_fd) {
                //printf("EVENT FROM listenfd\n");
                connect_fd = deal_accept(listen_fd);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connect_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &ev) == -1) {
                    printf ("error epoll_ctl, add\n");
                    continue;
                }
            } else if (events_[i].events&EPOLLHUP) {
                printf ("epoll hup.\n");
                close(events_[i].data.fd);
            } else if (events_[i].events&EPOLLERR) {
                printf ("epoll error.\n");
                close(events_[i].data.fd);
            } else {
                deal_with_fd(events_[i].data.fd);
            }
        }
    }
    return 0;
}

bool SERVER::Init(int backlog, int epoll_queue_size, int listen_port) 
{
    backlog_ = backlog;
    listen_port_ = listen_port;
    epoll_queue_size_ = epoll_queue_size;

    events_ = (struct epoll_event*) malloc(epoll_queue_size * sizeof(struct epoll_event));
    if (NULL == events_) {
        return -1;
    }

    return 0;
}

bool SERVER::AddWorker(WORKER* worker)
{
    workers_.push_back(worker);
    return true;
}

/*Network involved tools functions*/
void SERVER::setnonblocking(int fd)
{
    int opt = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, opt|O_NONBLOCK) == -1) {
        perror("fcntl set failed");
        exit(-1);
    }
}

int SERVER::deal_listen(int port, int backlog)
{
    int listen_fd;
    int sock_buf_size;

    struct sockaddr_in servaddr;
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("open socket file error");
        exit(-1);
    }

    int flag = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&flag, sizeof(flag)) == -1) {
        setsockopt(listen_fd, SOL_SOCKET, SO_RCVBUF, (char*)&sock_buf_size, sizeof(sock_buf_size)); 
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind error");
        exit(-1);
    }

    if (listen(listen_fd, backlog) < 0) {
        perror("listen error");
        exit(-1);
    }
    return listen_fd;
}

int SERVER::deal_accept(int listen_fd)
{
    int connect_fd;
    struct sockaddr_in client_addr;
    socklen_t addr_len;

    addr_len= sizeof(client_addr);

    if ((connect_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len)) < 0) {
        perror("accept error");
        exit(-1);
    } 

    printf("Client IP:%s Port:%d sockfd:%d\n", 
            inet_ntoa(client_addr.sin_addr), 
            client_addr.sin_port, connect_fd);

    setnonblocking(connect_fd);
    return connect_fd;
}

/* FIXME:isomorphism MODE 
 * just use the first one..
 */
void SERVER::deal_with_fd(int conn_sock)
{
    std::list <WORKER*>::iterator it = workers_.begin(); 
    for ( ; it!=workers_.end(); ++it) {
        (*it)->jobs_.inject(conn_sock, NULL, 0);
    }
    return;
}

}; //namespace kvs
