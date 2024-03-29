#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <stdio.h>
#include "http_state.h"

#define HTTP_METHOD_MAX (7)
#define HTTP_URI_MAX (255)

struct http_request
{
    char method[HTTP_METHOD_MAX + 1];
    char uri[HTTP_URI_MAX + 1];

    size_t uri_len;
    size_t method_len;

    const char *error;

    int state;
};

int http_request_ready(struct http_request *request);
void http_request_parse(struct http_request *request, char c);
void http_request_print(struct http_request *request);

#endif /* HTTP_REQUEST_H */
