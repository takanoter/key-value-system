#include <stdio.h>
#include <stdlib.h>
#include "src/client/client.h"

int main(int argc, char **argv)
{
    int fd = 0;
    char send_buffer[1024];

    if (argc<3) {
        printf ("need more param.\n");
        printf ("param: a send message,  port\n");
        return -1;
    }

    memset(send_buffer, 0, sizeof(send_buffer));
    sprintf(send_buffer, "%s", argv[1]);
    printf ("send buffer:[%s]\n", send_buffer);

    int port;
    sscanf(argv[2], "%d", &port);
    fd = connect_to_server("127.0.0.1", port);

    send_message_to_server(fd, send_buffer, strlen(send_buffer)+1); 

    sleep(5);
    close_connection(fd);
    return 0;
}
