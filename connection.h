#ifndef PLXR_CONNECTION_H
#define PLXR_CONNECTION_H

#include <arpa/inet.h>
#include <time.h>
#include <poll.h>
#include <netdb.h>

#include "socket.h"
#include "http_request.h"

struct connection
{
	int fd;

	struct sockaddr addr;

	char host[NI_MAXHOST];
    char port[NI_MAXSERV];

	struct http_request request;

	/* has the client sent end of file? */
	char eof;

	char shutdown;

	/* have we encountered a fatal error? */
	char error;

	/* have we wrote our response? */
	char wrote;

	/* how many times have we tried to write? */
	size_t write_attempts;

	/* how many times have we tried to read? */
	size_t read_attempts;

	/* currently only used for logging */
	int resp_status_code;
	time_t resp_timestamp;
	size_t resp_len;
};

void connection_accept(struct connection *conn, int listen_fd);
void connection_read(struct connection *conn);
void connection_write(struct connection *conn);
void connection_close(struct connection *conn);
void connection_http_parse(struct connection *conn, char c);

#endif /* PLXR_CONNECTION_H */
