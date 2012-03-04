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

#include <kvs.h>

#define MAXEVENTS			        1024
#define MAXEPOLLS			        128
#define	SYNC_TIME_SEC			    600
#define	SERVER_LAZY_TIME_MSEC		(300*1000)
#define	BACKLOG				        256

static int listen_port = 29866; 
static int deal_accept(int listen_fd);
static int deal_listen(int port);
static void setnonblocking(int fd);

static void do_use_fd(int conn_sock)
{
    char buf[1024];
    int n;
    int i;

    n = read(conn_sock, buf, sizeof(buf));
    for (i=0; i<n; i++) 
    {
        printf ("[%c]", buf[i]);
    }
    printf ("\n");
    write(conn_sock, "haha", sizeof("haha"));
}


int main(int argc, char** argv)
{
	int listen_fd, connect_fd, epoll_fd, nfds, i;
	struct epoll_event ev, events[MAXEVENTS];
	char c;

/*
	while ((c = getopt(argc, argv, "p:i:b:")) != -1) {
		switch (c) {
		case 'p':
			listen_port = atoi(optarg);
			break;
		case 'i':
			if (strcmp(optarg, "load") == 0) {
				kvs_env.init_type = KVS_LOAD;
			} else {
				kvs_env.init_type = KVS_CREATE;
			}
			break;
		case 'b':
			printf ("big file optarg:%s \n", optarg);
			kvs_env.disk_file = optarg;
			break;
		default:
			printf ("Illegal arguement: %c\n", c);
			exit (-1);
		}
	}
*/
			
	printf ("listen_port:%d\n", listen_port);
	listen_fd = deal_listen(listen_port);
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

	while (1) {
		printf ("listen_fd:%d, epoll_fd:%d\n", listen_fd, epoll_fd);
		nfds = epoll_wait(epoll_fd, events, MAXEVENTS, SERVER_LAZY_TIME_MSEC); 
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
					exit(0);
				}
			} else if (events[i].events&EPOLLHUP) {
				printf ("epoll hup.\n");
				close(events[i].data.fd);
			} else if (events[i].events&EPOLLERR) {
				printf ("epoll error.\n");
				close(events[i].data.fd);
			} else {
				do_use_fd(events[i].data.fd);
			}
		}
	}

	return 0;
}

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
/*
static void sig_usr(int signo)
{
	if (signo==SIGUSR1) {
		printf ("exit..\n");
		kv_exit();
	}
	return;
}
*/
