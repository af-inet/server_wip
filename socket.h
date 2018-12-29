#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>

int socket_listen(char *host, char *port);

char *socket_error(int fd);

void socket_ntop(struct sockaddr *src, char dest[INET_ADDRSTRLEN]);

#endif /* SOCKET_H */
