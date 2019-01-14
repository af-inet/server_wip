#include "connection_list.h"

void connection_list_init(struct connection_list *list)
{
    for (size_t i = 0; i < MAX_CONNECTIONS; i++)
    {
        list->poll_fds[i] = (struct pollfd) {.fd = -1};
        list->connections[i] = (struct connection) {.fd = -1};
    }
    list->count = 0;
    list->total_count = 0;
}

int connection_add(struct connection_list *list, struct connection conn)
{
    if (list->count >= MAX_CONNECTIONS)
    {
        WARNF("exceeded max connections (%d)", MAX_CONNECTIONS);
        return -1;
    }
    list->connections[list->count] = conn;
    list->poll_fds[list->count] = (struct pollfd){.fd = conn.fd, .events = POLLIN | POLLOUT};
    list->count += 1;
    return 0;
}

int connection_remove(struct connection_list *list, size_t index)
{
    if (list->count == 0)
    {
        WARN("empty list");
        return -1;
    }
    if (index > list->count)
    {
        WARN("index out of bounds");
        return -1;
    }
    // replace the connection at `index` with the last connection
    list->connections[index] = list->connections[list->count - 1];
    list->poll_fds[index] = list->poll_fds[list->count - 1];
    // just to avoid potentially using old file descriptors, lets set them to -1
    list->connections[list->count - 1] = (struct connection){.fd = -1};
    list->poll_fds[list->count - 1] = (struct pollfd){.fd = -1};
    // we have 1 less connection now
    list->count -= 1;
    return 0;
}
