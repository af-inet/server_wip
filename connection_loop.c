#include "connection_loop.h"

void connection_loop(struct connection_list *list)
{
    int count = poll(list->poll_fds, list->count, 0);

    switch (count)
    {
    case -1:
        ERROR("poll");
        return;
    case 0:
        WARN("poll timeout");
        return;
    default:
        break;
    }
    for (size_t i = 0; i < list->count; i++)
    {
        struct connection *conn = &list->connections[i];
        short events = list->poll_fds[i].revents;
        if (
            (events & (POLLHUP | POLLERR | POLLNVAL)) ||
            conn->error ||
            conn->wrote)
        {
            if (events & POLLHUP)
            {
                WARNF("(%s:%s) %s", conn->host, conn->port, "POLLHUP");
            }
            if (events & POLLNVAL)
            {
                WARNF("%d (%s:%s) %s", conn->fd, conn->host, conn->port, "POLLNVAL");
            }
            if (events & POLLERR)
            {
                WARNF("(%s:%s) %s", conn->host, conn->port, socket_error(conn->fd));
            }
            if (conn->error)
            {
                WARNF("(%s:%s) %s", conn->host, conn->port, "error");
            }
            if (conn->wrote)
            {
            }
            connection_close(conn);
            connection_remove(list, i--);
        }
        else
        {
            if (events & POLLIN)
            {
                connection_read(conn);
            }
            if (events & POLLOUT)
            {
                connection_write(conn);
            }
        }
    }
}
