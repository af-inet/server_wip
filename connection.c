#include <string.h>
#include <fcntl.h>
#include "connection.h"
#include "http_request.h"
#include "log.h"

void connection_accept(struct connection *conn, int listen_fd)
{
    socklen_t socklen = sizeof(conn->addr);

    memset(conn, 0, sizeof(struct connection));

    conn->fd = accept(listen_fd, &conn->addr, &socklen);

    int err = getnameinfo(
        &conn->addr, socklen,             /* (input) address */
        conn->host, sizeof(conn->host),   /* (output) host */
        conn->port, sizeof(conn->port),   /* (output) port */
        NI_NUMERICSERV | NI_NUMERICHOST); /* (input) give us the numeric hostname/port info */

    if (err)
    {
        GAI_ERROR("getnameinfo", err);
        conn->fd = -1;
    }

    err = fcntl(conn->fd, F_SETFL, O_NONBLOCK);
    if (err == -1)
    {
        ERROR("fnctl");
        close(conn->fd);
        conn->fd = -1;
    }
}

void connection_read(struct connection *conn)
{
    char buf[1024];
    int count;
    ssize_t i;
    bzero(buf, sizeof(buf));

    count = read(conn->fd, buf, sizeof(buf));

    if (count < 0)
    {
        switch (errno)
        {
        case ETIMEDOUT:
        case EAGAIN:
            /* try again later */
            WARNF("(%s:%s) EAGAIN or ETIMEDOUT", conn->host, conn->port);
            break;
        default:
            /* read failed, assume the socket is broken */
            ERRORF("read", "(%s:%s)", conn->host, conn->port);
            /* mark this connection as broken */
            conn->error = 1;
            break;
        }
    }
    else if (count == 0)
    {
        /* reached EOF, the client is done sending data */
        WARNF("(%s:%s) EOF", conn->host, conn->port);
        conn->eof = 1;
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            http_request_parse(&conn->request, buf[i]);
            /* check for parsing errors */
            if (conn->request.state == s_error)
            {
                WARNF("(%s:%s) http_request_parse error: %s",
                      conn->host, conn->port, http_state_string(conn->request.state));
                /* mark this connection as broken */
                conn->error = 1;
                break;
            }
        }
    }
}

void connection_write(struct connection *conn)
{
    if (!conn->wrote && http_request_ready(&conn->request))
    {
        ssize_t count;
        const char *msg =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n" // don't reuse connections (good way to test EOF handling)
            "Content-Length: 10\r\n"
            "\r\n"
            "123456789\n";

        ssize_t msg_len = strlen(msg);

        count = write(conn->fd, msg, msg_len);

        if (count < 0)
        {
            switch (errno)
            {
            case ETIMEDOUT:
            case EAGAIN:
                /* try again later */
                WARNF("EAGAIN (%s:%s)", conn->host, conn->port);
                break;
            default:
                ERRORF("write", "(%s:%s)", conn->host, conn->port);
                break;
            }
        }
        else if (count != msg_len)
        {
            WARNF("wrote incorrect number of bytes (%s:%s)", conn->host, conn->port);
            /* mark this connection as broken */
            conn->error = 1;
        }
        else
        {
            conn->wrote = 1;
        }
    }
}

void connection_close(struct connection *conn)
{
    shutdown(conn->fd, SHUT_RDWR);
    close(conn->fd);
}
