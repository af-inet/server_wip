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

void wait_for_connections(struct connection_list *list, int listen_fd, int timeout)
{
    struct connection new_conn = {0};
    struct pollfd pfd = {
        .fd = listen_fd,
        .events = POLLIN,
        .revents = 0
    };
    int count = poll(&pfd, 1, timeout);

    switch (count)
    {
    case 0:
        break;
    case -1:
        ERROR("poll");
        break;
    default:
        for (int i = 0; i < count; i++)
        {
            connection_accept(&new_conn, listen_fd);
            if (new_conn.fd != -1)
            {
                connection_add(list, new_conn);
            }
            else
            {
                WARN("connection_accept: failed");
            }
        }
    }
}

int main(int argc, char *argv[])
{
    struct connection_list list[1] = {0};

    /* ignore SIGPIPE */
    signal(SIGPIPE, sigpipe_handler);

    /* disable buffered output */
    setvbuf(stdout, NULL, _IONBF, 0);

    int listen_fd = socket_listen("0.0.0.0", "8080");

    while (1)
    {
        if (list->count == 0)
        {
            wait_for_connections(list, listen_fd, -1);
        }
        else
        {
            wait_for_connections(list, listen_fd, 0);
        }
        connection_loop(list);
    }

    return 0;
}
