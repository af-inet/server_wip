#ifndef CONNECTION_LIST_H
#define CONNECTION_LIST_H

#include <poll.h>
#include "connection.h"
#include "log.h"

#define MAX_CONNECTIONS (1024)

struct connection_list
{
    struct pollfd poll_fds[MAX_CONNECTIONS];
    struct connection connections[MAX_CONNECTIONS];
    size_t count;
    size_t total_count;
};

void connection_list_init(struct connection_list *list);
int connection_add(struct connection_list *list, struct connection conn);
int connection_remove(struct connection_list *list, size_t index);

#endif /* CONNECTION_LIST_H */
