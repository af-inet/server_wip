#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <stdlib.h>
#include <stdio.h>

struct http_response
{
    unsigned short status_code;
    const char *content_type;
    size_t content_length;
    char *content;
};

int http_response_format(struct http_response *resp, char *dest, size_t count);

#endif /* HTTP_RESPONSE_H */
