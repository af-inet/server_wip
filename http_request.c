#include "http_request.h"

void http_request_parse_uri(struct http_request *request, char c)
{
    if (request->uri_len >= HTTP_URI_MAX)
    {
        request->error = "uri too long";
        request->state = s_http_error;
    }
    else
    {
        request->uri[request->uri_len] = c;
        request->uri_len += 1;
    }
}

void http_request_parse_method(struct http_request *request, char c)
{
    if (request->method_len >= HTTP_METHOD_MAX)
    {
        request->error = "method too long";
        request->state = s_http_error;
    }
    else
    {
        request->method[request->method_len] = c;
        request->method_len += 1;
    }
}

void http_request_parse(struct http_request *request, char c)
{
    request->state = http_state_next(request->state, c);

    switch (request->state)
    {
    case s_http_method:
        http_request_parse_method(request, c);
        break;
    case s_http_uri:
        http_request_parse_uri(request, c);
        break;
    default:
        break;
    }
}

int http_request_ready(struct http_request *request)
{
    switch (request->state)
    {
        case s_http_end_cr:
        case s_http_end_lf:
            return 1;
        default:
            return 0;
    }
}

void http_request_print(struct http_request *request)
{
    if (request->error)
    {
        printf("error = '%s'\n", request->error);
    }
    printf("method = '%.*s'\n", (int)request->method_len, request->method);
    printf("uri = '%.*s'\n", (int)request->uri_len, request->uri);
    printf("state = '%s'\n", http_state_string(request->state));
}
