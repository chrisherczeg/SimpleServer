#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "server.c"


int main(int argc, char **argv) {

    int mode = atoi(argv[1]);
    int port = atoi(argv[2]);

    int master_socket = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR,
                &optval,  sizeof(optval));

    socklen_t length = sizeof(addr);
    bind(master_socket, (struct sockaddr *) &addr, length);

    listen(master_socket, 50);

    switch(mode) {
        case 0:
            printf("\nRunning on Linear Server Port: %d\n", port);
            run_linear_server(master_socket);
            break;
        case 1:
            printf("\nRunning on run_forking_server Port: %d\n", port);
            run_forking_server(master_socket);
            break;
        case 2:
            printf("\nRunning on run_threaded_server Port: %d\n", port);
            run_threaded_server(master_socket);
            break;
        case 3:
            printf("\nRunning on run_thread_pool_server Port: %d\n", port);
            run_thread_pool_server(master_socket);
            break;
        default:
            break;
    }
}