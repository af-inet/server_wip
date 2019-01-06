#ifndef CONNECTION_LOOP_H
#define CONNECTION_LOOP_H

#include "connection_list.h"
#include "connection.h"
#include "socket.h"

int connection_loop(struct connection_list *list, int listen_fd);

#endif /* CONNECTION_LOOP_H */
