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
#define SERVER_PORT 29866
#define SERVER_IP   "127.0.0.1"
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

int main(int argc, char **argv)
{
/*
    if (argc != 2)
    {
        printf("Please input the IP address of the server \n", argv[0]);
        exit(1);
    }
*/
    int ret;
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr)); 
    client_addr.sin_family = AF_INET; 
    client_addr.sin_addr.s_addr = htons(INADDR_ANY); 
    client_addr.sin_port = htons(0); 
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
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

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_aton(SERVER_IP, &server_addr.sin_addr) == 0) 
    {
        printf("Server IP Address Error! \n");
        exit(1);
    }

    server_addr.sin_port = htons(SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);

    ret = connect(client_socket, (struct sockaddr*) &server_addr, server_addr_length);
    if (ret < 0)
    {
        printf("Can Not Connect To %s! error:%s\n", SERVER_IP, strerror(errno));
        exit(1);
    }

    send(client_socket, "haha", sizeof("haha"), 0);

    sleep(5);
    close(client_socket);
    return 0;
}
