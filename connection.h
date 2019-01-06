#ifndef CONNECTION_H
#define CONNECTION_H

#include <arpa/inet.h>
#include <time.h>
#include <poll.h>
#include <netdb.h>

#include "socket.h"
#include "http_request.h"

enum connection_state
{
    s_conn_read,  /* reading request */
    s_conn_write, /* writing response */
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

    char host[NI_MAXHOST];
    char port[NI_MAXSERV];

    struct http_request request;

    enum connection_state state;

    size_t bytes_wrote;
    size_t bytes_read;

    /* currently only used for logging */
    int resp_status_code;
    time_t resp_timestamp;
    size_t resp_len;
};

void connection_accept(struct connection *conn, int listen_fd);
void connection_close(struct connection *conn);
enum connection_state connection_update(struct connection *conn);
const char *connection_state_string(enum connection_state s);
void connection_debug(struct connection *conn);

// void connection_read(struct connection *conn);
// void connection_write(struct connection *conn);
// void connection_close(struct connection *conn);
// void connection_http_parse(struct connection *conn, char c);

#endif /* CONNECTION_H */
