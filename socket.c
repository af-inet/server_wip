#include "socket.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <arpa/inet.h>

#include "log.h"

int socket_listen(char *host, char *port)
{
    int err;
    int fd;
    struct addrinfo *info;
    int yes = 1;
    struct addrinfo hints = {.ai_flags = AI_PASSIVE};

    err = getaddrinfo(host, port, &hints, &info);
    if (err)
    {
        GAI_ERROR("getaddrinfo", err);
        return -1;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        ERROR("socket");
        return -1;
    }

    err = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (err)
    {
        ERROR("setsockopt");
        close(fd);
        return -1;
    }

    // err = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
    // if (err)
    // {
    //     ERROR("setsockopt");
    //     close(fd);
    //     return -1;
    // }

    err = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (err == -1)
    {
        ERROR("fnctl");
        close(fd);
        return -1;
    }

    err = bind(fd, info->ai_addr, info->ai_addrlen);
    if (err)
    {
        ERROR("bind");
        close(fd);
        return -1;
    }

    err = listen(fd, 128);
    if (err)
    {
        ERROR("listen");
        close(fd);
        return -1;
    }

    return fd;
}

char *socket_error(int fd)
{
    int opt;
    socklen_t opt_len = sizeof(opt);
    int err = getsockopt(fd, SOL_SOCKET, SO_ERROR, &opt, &opt_len);
    if (err)
    {
        ERROR("getsockopt");
        return "setsockopt error";
    }
    else
    {
        return strerror(opt);
    }
}
