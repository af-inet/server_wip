#include <string.h>
#include <fcntl.h>
#include "connection.h"
#include "http_request.h"
#include "log.h"

void connection_accept(struct connection *conn, int listen_fd)
{
    int err;
    socklen_t socklen = sizeof(conn->addr);

    memset(conn, 0, sizeof(*conn));

    conn->fd = accept(listen_fd, &conn->addr, &socklen);
    if (conn->fd == -1)
    {
        ERROR("accept");
        return;
    }

    err = getnameinfo(
        &conn->addr, socklen,             /* (input) address */
        conn->host, sizeof(conn->host),   /* (output) host */
        conn->port, sizeof(conn->port),   /* (output) port */
        NI_NUMERICSERV | NI_NUMERICHOST); /* (input) give us the numeric hostname/port info */
    if (err)
    {
        GAI_ERROR("getnameinfo", err);
        close(conn->fd);
        conn->fd = -1;
        return;
    }

    err = fcntl(conn->fd, F_SETFL, O_NONBLOCK);
    if (err == -1)
    {
        ERROR("fnctl");
        close(conn->fd);
        conn->fd = -1;
        return;
    }
}

void connection_close(struct connection *conn)
{
    shutdown(conn->fd, SHUT_RDWR);
    close(conn->fd);
}

int connection_parse(struct connection *conn, char *buf, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        http_request_parse(&conn->request, buf[i]);
        if (conn->request.state == s_http_error)
        {
            return -1;
        }
    }
    return 0;
}

enum connection_state connection_state_write(struct connection *conn)
{
    ssize_t count;
    const char msg[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n" // don't reuse connections (good way to test EOF handling)
        "Content-Length: 10\r\n"
        "\r\n"
        "123456789\n";
    ssize_t msg_len = sizeof(msg) - 1;
    
    if (!conn->writable)
    {
        return conn->state;
    }

    count = write(conn->fd, msg, msg_len);

    if (count < 0)
    {
        if (errno == EAGAIN)
        {
            return s_conn_write; /* retry */
        }
        else
        {
            ERRORF("write", "%d %s:%s", conn->fd, conn->host, conn->port);
            return s_conn_error;
        }
    }
    else if (count != msg_len)
    {
        WARNF("wrote incorrect number of bytes %4d %s:%s", conn->fd, conn->host, conn->port);
        return s_conn_error;
    }
    else
    {
        conn->bytes_wrote += count;
        return s_conn_wait;
    }
}

enum connection_state connection_state_read(struct connection *conn)
{
    int count;
    int err;
    static char buf[1024];

    if (!conn->readable)
    {
        return conn->state;
    }

    memset(buf, 0, sizeof(buf));

    count = read(conn->fd, buf, sizeof(buf));

    if (count < 0)
    {
        if (errno == EAGAIN)
        {
            return s_conn_read; /* retry */
        }
        else
        {
            ERRORF("read", "%4d %s:%s", conn->fd, conn->host, conn->port);
            return s_conn_error;
        }
    }
    else if (count == 0)
    {
        WARNF("EOF %4d %s:%s", conn->fd, conn->host, conn->port);
        return s_conn_error;
    }
    else
    {
        conn->bytes_read += count;
        
        err = connection_parse(conn, buf, count);
        
        if (err == -1)
        {
            WARNF("connection_parse error %d %s:%s", conn->fd, conn->host, conn->port);
            return s_conn_error;
        }
        else if (http_request_ready(&conn->request))
        {
            return s_conn_write;
        }
        else
        {
            return s_conn_read;
        }
    }
}

enum connection_state connection_state_wait(struct connection *conn)
{
    int count;
    char buf[1];

    if (!conn->readable)
    {
        return conn->state;
    }

    count = read(conn->fd, buf, sizeof(buf));

    if (count < 0)
    {
        if (errno == EAGAIN)
        {
            return s_conn_wait; /* retry */
        }
        else
        {
            ERRORF("read", "%d %s:%s", conn->fd, conn->host, conn->port);
            return s_conn_error;
        }
    }
    else if (count == 0)
    {
        return s_conn_eof; /* success */
    }
    else
    {
        return s_conn_wait; /* received extra data, not sure what to do with this */
    }
}

enum connection_state connection_update(struct connection *conn)
{
    switch (conn->state)
    {
    case s_conn_read:
        return connection_state_read(conn);
    case s_conn_write:
        return connection_state_write(conn);
    case s_conn_wait:
        return connection_state_wait(conn);
    case s_conn_eof:
        return s_conn_eof;
    case s_conn_error:
        return s_conn_error;
    }
}
