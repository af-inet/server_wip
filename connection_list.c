#include "connection_list.h"

struct connection_list connection_list()
{
    return (struct connection_list) {
        0
    };
}

int connection_add(struct connection_list *list, struct connection conn)
{
    if (list->count >= MAX_CONNECTIONS)
    {
        WARN("ran out of space, no where to put connection");
        return -1;
    }
    list->connections[list->count] = conn;
    list->poll_fds[list->count] = (struct pollfd){.fd = conn.fd, .events = POLLIN | POLLOUT, .revents = 0};
    list->count += 1;
    return 0;
}

int connection_remove(struct connection_list *list, size_t index)
{
    if (list->count == 0)
    {
        WARN("attempted to remove from empty list");
        return -1;
    }
    if (index > list->count)
    {
        WARN("attempted to remove a non-existant connection");
        return -1;
    }
    if (list->count > 1)
    {
        // replace `index` with the last item in the list
        list->connections[index] = list->connections[list->count - 1];
        list->poll_fds[index] = list->poll_fds[list->count - 1];
    }
    // clear out the old poll structure
    list->poll_fds[list->count - 1] = (struct pollfd){.fd = -1, .events = 0, .revents = 0};
    list->count -= 1;
    return 0;
}
