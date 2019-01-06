#include <stdio.h>
#include <signal.h>
#include "http_request.h"
#include "socket.h"
#include "connection_list.h"
#include "connection_loop.h"

void sigpipe_handler(int sig)
{
    printf("[!] caught SIGPIPE\n");
}

int main(int argc, char *argv[])
{
    struct connection_list list[1] = {0};
    char port[] = "8080";

    signal(SIGPIPE, sigpipe_handler);

    int listen_fd = socket_listen("0.0.0.0", port);

    if (listen_fd == -1)
    {
        WARN("socket_listen: failed");
        return 1;
    }

    WARNF("listening on port: %s", port);

    connection_list_init(list);

    while (connection_loop(list, listen_fd) != -1)
        ; /* keep going until we reach a fatal error */
    
    return 0;
}
