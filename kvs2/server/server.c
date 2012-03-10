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

static int deal_accept(int listen_fd);
static int deal_listen(int port);
static void setnonblocking(int fd);
static void do_use_fd(KV_SERVER* server, int conn_sock);
static int server_check(KV_SERVER* server);

int server_run(KV_SERVER* server)
{
	struct epoll_event ev;
    int i = 0;
    int listen_fd = 0;
    int connect_fd;
    if (NULL == server) {
        return -1;
    }
    if (0 !== server_check(server)) {
        return -1;
    }

	listen_fd = deal_listen(server->listen_port);
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
    
    WORKER_INFO worker_info; /*TODO: worker_info_array[] ? */
    worker_info.worker_thread_func = server.worker_thread_func;
    worker_info.server = server;
    for (i = 0; i < server->worker_thread_num; i++) {
        ptrhead_create(&worker_thread_id[i], NULL, worker, &worker_info);
    }
    server_thread_init(server);

	while (1) {
		printf ("listen_fd:%d, epoll_fd:%d\n", listen_fd, epoll_fd);
		nfds = epoll_wait(epoll_fd, events, server->epool_queue_size, server->fresh_msec); 
		if (nfds == -1) {
			perror("epoll_wait error");
			exit(-1);
		}

		for (i = 0; i < nfds; i++) {
			if (events[i].data.fd == listen_fd) {
				printf("EVENT FROM listenfd\n");
				connect_fd = deal_accept(listen_fd);

				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = connect_fd;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &ev) == -1) {
					printf ("error epoll_ctl, add\n");
                    continue;
				}
			} else if (events[i].events&EPOLLHUP) {
				printf ("epoll hup.\n");
				close(events[i].data.fd);
			} else if (events[i].events&EPOLLERR) {
				printf ("epoll error.\n");
				close(events[i].data.fd);
			} else {
				do_use_fd(server, events[i].data.fd);
			}
		}
	}
    return 0;
}

int server_set_queue_size(KV_SERVER* server, const int backlog, const int epool_queue_size, const int job_queue_size)
{
    int ret = 0;
    if (NULL == server) {
        return -1;
    }
    server->backlog = backlog;

    server->epool_queue_size = epool_queue_size;
    server->events = malloc(epool_queue_size * sizeof(struct epoll_event));
    if (NULL == server->events) {
        return -1;
    }

    ret = jobs_init(&(server->jobs), job_queue_size);
    if (0 != ret) {
        printf ("jobs init failed.\n");
        return -1;
    }

    /*TODO: memory leak*/
    return 0;
}

int server_set_thread(KV_SERVER* server, const int worker_thread_num, WORKER_FUNC worker_thread_func)
{
    if (NULL == server) {
        return -1;
    }
    server->worker_thread_num = worker_thread_num;
    server->worker_thread_func = worker_thread_func;
    server->worker_thread_id = malloc(worker_thread_num * sizeof(pthread_t));
    if (NULL == server->worker_thread_id) {
        return -1;
    }
    
    return 0;
}

int server_set_timeout(KV_SERVER* server, const int fresh_msec)
{
    if (NULL == server) {
        return -1;
    }
    server->fresh_msec = fresh_msec;
    return 0;
}

int server_set_port(KV_SERVER* server, const int listen_port)
{
    if (NULL == server) {
        return -1;
    }
    server->listen_port = listen_port;
    return 0;
}

/*Server bind functions*/
static void do_use_fd(KV_SERVER* server, int conn_sock)
{
    jobs_push(server->jobs, conn_sock);
}

/*Network involved tools functions*/
static void setnonblocking(int fd)
{
	int opt = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, opt|O_NONBLOCK) == -1) {
		perror("fcntl set failed");
		exit(-1);
	}
}

static int deal_listen(int port)
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

	if (listen(listen_fd, BACKLOG) < 0) {
		perror("listen error");
		exit(-1);
	}
	return listen_fd;
}

static int deal_accept(int listen_fd)
{
	int connect_fd;
	struct sockaddr_in client_addr;
	socklen_t addr_len;

	addr_len= sizeof(client_addr);

	if ((connect_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len)) < 0) {
		perror("accept error");
		exit(-1);
	} 

	printf("Client IP:%s Port:%d sockfd:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, connect_fd);

	setnonblocking(connect_fd);
	return connect_fd;
}

static int server_check(KV_SERVER *server)
{
    /*TODO*/
    printf ("server check success.\n");
    return 0;
}
