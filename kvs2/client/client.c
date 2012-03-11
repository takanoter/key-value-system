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

int connect_to_server(char* ip, const int port)
{
    int ret;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    int client_socket;

    if (NULL == ip) {
        printf ("ip is NULL.\n");
        return -1;
    }

    bzero(&client_addr, sizeof(client_addr)); 
    client_addr.sin_family = AF_INET; 
    client_addr.sin_addr.s_addr = htons(INADDR_ANY); 
    client_addr.sin_port = htons(0); 
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }

    if (bind(client_socket, (struct sockaddr*) &client_addr,
            sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n");
        exit(1);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_aton(ip, &server_addr.sin_addr) == 0) 
    {
        printf("Server IP Address Error! \n");
        exit(1);
    }

    server_addr.sin_port = htons(port);
    socklen_t server_addr_length = sizeof(server_addr);

    ret = connect(client_socket, (struct sockaddr*) &server_addr, server_addr_length);
    if (ret < 0)
    {
        printf("Can Not Connect To %s! error:%s\n", ip, strerror(errno));
        exit(1);
    }
    return client_socket;
}

int send_message_to_server(int fd, void* buf, int buf_size)
{
    if (fd < 0) {
        printf ("invalid fd:%d\n", fd);
    }
    send(fd, buf, buf_size, 0);
}

int close_connection(int fd)
{
    close(fd);
}
