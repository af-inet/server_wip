#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include "connection.h"
#include "http_request.h"
#include "http_response.h"
#include "file.h"
#include "log.h"
#include "result.h"

static const char html_404[] =
    "<html><head></head><body><h1>file not found</h1></body></html>";

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

void configure_response(struct connection *conn)
{
    struct file_data f = file_get(conn->request.uri);
    if (f.data == NULL)
    {
        conn->response = (struct http_response){
            .status_code = 404,
            .content = (char *)html_404,
            .content_type = "text/html",
            .content_length = sizeof(html_404)};
    }
    else
    {
        conn->response = (struct http_response){
            .status_code = 200,
            .content = f.data,
            .content_type = "text/html",
            .content_length = f.size};
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
            configure_response(conn);
            return s_conn_write_header;
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

void write_response_headers(struct connection *conn)
{
    char buffer[1024];
    int count = http_response_format(&conn->response, buffer, sizeof(buffer));
    conn->stream.data = buffer;
    conn->stream.length = (size_t)count;
    stream_write(&conn->stream, conn->fd);
}

void write_response_body(struct connection *conn)
{
    conn->stream.data = conn->response.content;
    conn->stream.length = conn->response.content_length;
    stream_write(&conn->stream, conn->fd);
}

enum connection_state connection_state_write_header(struct connection *conn)
{
    if (!conn->writable)
    {
        return conn->state;
    }
    write_response_headers(conn);
    switch (conn->stream.state)
    {
    case OK:
        return s_conn_write_header;
    case DONE:
        stream_reset(&conn->stream);
        return s_conn_write_body;
    case FAILED:
        return s_conn_error;
    }
}

enum connection_state connection_state_write_body(struct connection *conn)
{
    if (!conn->writable)
    {
        return conn->state;
    }
    write_response_body(conn);
    switch (conn->stream.state)
    {
    case OK:
        return s_conn_write_body;
    case DONE:
        stream_reset(&conn->stream);
        return s_conn_wait;
    case FAILED:
        return s_conn_error;
    }
}

enum connection_state connection_update(struct connection *conn)
{
    switch (conn->state)
    {
    case s_conn_read:
        return connection_state_read(conn);
    case s_conn_write_header:
        return connection_state_write_header(conn);
    case s_conn_write_body:
        return connection_state_write_body(conn);
    case s_conn_wait:
        return connection_state_wait(conn);
    case s_conn_eof:
        return s_conn_eof;
    case s_conn_error:
        return s_conn_error;
    }
}
