#ifndef KV_CLIENT_H
#define KV_CLIENT_H 1

#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>

int connect_to_server(char* ip, const int port);

int send_message_to_server(int fd, void* buf, int buf_size);

int close_connection(int fd);

#endif
