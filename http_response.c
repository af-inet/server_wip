#include "http_response.h"

const char *HTTP_RESP_FORMAT =
    "HTTP/1.1 %d %s\r\n"
    "Content-Type: %s\r\n"
    "Connection: close\r\n" // tell clients not to reuse connections
    "Content-Length: %d\r\n"
    "\r\n"
    ;

const char *http_phrase(int status_code) {
    switch(status_code) {
    case 100: return "Continue";
    case 101: return "Switching Protocols";
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-Authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 307: return "Temporary Redirect";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Time-out";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Request Entity Too Large";
    case 414: return "Request-URI Too Large";
    case 415: return "Unsupported Media Type";
    case 416: return "Requested range not satisfiable";
    case 417: return "Expectation Failed";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Time-out";
    case 505: return "HTTP Version not supported";
    default: return "Invalid Status Code";
    }
}

int http_response_format(struct http_response *resp, char *dest, size_t count)
{
    return snprintf(
        dest,
        count,
        HTTP_RESP_FORMAT,
        resp->status_code,
        http_phrase(resp->status_code),
        resp->content_type,
        resp->content_length);
}
