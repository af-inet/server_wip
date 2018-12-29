#ifndef http_state_h
#define http_state_h

#include <stdio.h>

enum http_state
{
    s_init,
    s_method,
    s_method_sp,
    s_uri,
    s_uri_sp,
    s_h,
    s_ht,
    s_htt,
    s_http,
    s_http_slash,
    s_http_slash_1,
    s_http_slash_1_dot,
    s_http_slash_1_dot_x,
    s_status_cr,
    s_status_lf,
    s_header_name,
    s_header_colon,
    s_header_sp,
    s_header_value,
    s_header_cr,
    s_header_lf,
    s_end_cr,
    s_end_lf,
    s_error,
};

#define HTTP_STATE_COUNT (s_error + 1)

const char *http_state_string(enum http_state state);

enum http_state http_state_next(char last, char c);

#endif /* http_state_h */
