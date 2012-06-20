#include <stdio.h>
#include <stdlib.h>

#include "client.h"

int main(int argc, char **argv)
{
    int fd = 0;
    char send_buffer[1024];

    if (argc<2) {
        printf ("need more param.\n");
        printf ("param: a send message\n");
        return -1;
    }

    memset(send_buffer, 0, sizeof(send_buffer));
    sprintf(send_buffer, "%s", argv[1]);
    printf ("send buffer:[%s]\n", send_buffer);

    fd = connect_to_server("127.0.0.1", 38000);

    send_message_to_server(fd, send_buffer, strlen(send_buffer)+1); 

    sleep(5);
    close_connection(fd);
    return 0;
}
