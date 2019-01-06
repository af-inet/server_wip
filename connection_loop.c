#include "connection_loop.h"

int connection_loop_accept(struct connection_list *list, int listen_fd, int timeout)
{
    struct connection new_conn = {0};
    struct pollfd pfd = {
        .fd = listen_fd,
        .events = POLLIN,
        .revents = 0};
    int count = poll(&pfd, 1, timeout);

    switch (count)
    {
    case 0:
        break; /* timeout */
    case -1:
        ERROR("poll");
        return -1;
    default:
        connection_accept(&new_conn, listen_fd);
        if (new_conn.fd != -1)
        {
            list->total_count += 1;
            connection_add(list, new_conn);
            return 1;
        }
    }
    return 0;
}

void connection_loop_iterate(struct connection_list *list)
{
    for (size_t i = 0; i < list->count; i++)
    {
        struct connection *conn = &list->connections[i];
        conn->readable = list->poll_fds[i].events & POLLIN;
        conn->writable = list->poll_fds[i].events & POLLOUT;

        conn->state = connection_update(conn);

        switch (conn->state)
        {
        case s_conn_eof:
            printf("%s %s %s\n",
                   conn->host,
                   conn->request.method,
                   conn->request.uri);
            connection_close(&list->connections[i]);
            connection_remove(list, i);
            if (i > 0)
                i--;
            break;
        case s_conn_error:
            connection_close(&list->connections[i]);
            connection_remove(list, i);
            if (i > 0)
                i--;
            break;
        default:
            break;
        }
    }
}

int connection_loop_poll(struct connection_list *list, int listen_fd)
{
    int count = poll(list->poll_fds, list->count, 0);
    switch (count)
    {
    case -1:
        ERROR("poll");
        return -1;
    case 0:
        WARN("poll timeout");
        break;
    default:
        connection_loop_iterate(list);
        break;
    }
    return 0;
}

int connection_loop(struct connection_list *list, int listen_fd)
{
    int err;
    if (list->count == 0)
    {
        err = connection_loop_accept(list, listen_fd, -1);
        if (err == -1)
        {
            WARN("connection_loop_accept: error");
            return -1;
        }
    }
    else
    {
        while ((err = connection_loop_accept(list, listen_fd, 0)) == 1)
            ; /* keep accepting connectionas as long as their queued */
        if (err == -1)
        {
            WARN("connection_loop_accept: error");
            return -1;
        }
    }
    return connection_loop_poll(list, listen_fd);
}
