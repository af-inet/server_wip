#ifndef CONNECTION_H
#define CONNECTION_H

#include <arpa/inet.h>
#include <time.h>
#include <poll.h>
#include <netdb.h>

#include "socket.h"
#include "stream.h"
#include "http_request.h"
#include "http_response.h"

enum connection_state
{
    s_conn_read,  /* reading request */
    s_conn_write_header, /* writing response headers */
    s_conn_write_body, /* writing response body */
    s_conn_wait,  /* we're done writing, have not received EOF yet */
    s_conn_eof,   /* client gracefully disconnected */
    s_conn_error, /* an error occured */
};

struct connection
{
    int fd;
    char readable;
    char writable;

    struct sockaddr addr;

    /* "... enough space must be provided to store the host name or service string
    plus a byte for the NUL terminator." getnameinfo(3) */
    char host[NI_MAXHOST+1];
    char port[NI_MAXSERV+1];

    struct http_request request;
    struct http_response response;

    enum connection_state state;

    struct stream stream;

    size_t bytes_wrote;
    size_t bytes_read;

    /* currently only used for logging */
    time_t resp_timestamp;
};

void connection_accept(struct connection *conn, int listen_fd);
void connection_close(struct connection *conn);
enum connection_state connection_update(struct connection *conn);
const char *connection_state_string(enum connection_state s);
void connection_debug(struct connection *conn);

#endif /* CONNECTION_H */
