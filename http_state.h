#ifndef HTTP_STATE_H
#define HTTP_STATE_H

#include <stdio.h>

enum http_state
{
    s_http_init = 0,
    s_http_method,
    s_http_method_sp,
    s_http_uri,
    s_http_uri_sp,
    s_http_h,
    s_http_ht,
    s_http_htt,
    s_http_http,
    s_http_http_slash,
    s_http_http_slash_1,
    s_http_http_slash_1_dot,
    s_http_http_slash_1_dot_x,
    s_http_status_cr,
    s_http_status_lf,
    s_http_header_name,
    s_http_header_colon,
    s_http_header_sp,
    s_http_header_value,
    s_http_header_cr,
    s_http_header_lf,
    s_http_end_cr,
    s_http_end_lf,
    s_http_error,
};

const char *http_state_string(enum http_state state);

enum http_state http_state_next(enum http_state last, char c);

#endif /* HTTP_STATE_H */
