#include "http_state.h"

/* https://tools.ietf.org/html/rfc2616
 * https://tools.ietf.org/html/rfc1808
 * https://tools.ietf.org/html/rfc3986#
 */

const char *http_state_string(enum http_state state)
{
#define _STRING(x) \
    case (x):      \
        return #x;
    switch (state)
    {
        _STRING(s_init)
        _STRING(s_error)
        _STRING(s_method)
        _STRING(s_method_sp)
        _STRING(s_uri)
        _STRING(s_uri_sp)
        _STRING(s_h)
        _STRING(s_ht)
        _STRING(s_htt)
        _STRING(s_http)
        _STRING(s_http_slash)
        _STRING(s_http_slash_1)
        _STRING(s_http_slash_1_dot)
        _STRING(s_http_slash_1_dot_x)
        _STRING(s_status_cr)
        _STRING(s_status_lf)
        _STRING(s_header_name)
        _STRING(s_header_colon)
        _STRING(s_header_sp)
        _STRING(s_header_value)
        _STRING(s_header_cr)
        _STRING(s_header_lf)
        _STRING(s_end_cr)
        _STRING(s_end_lf)
    }
    return "unknown state";
#undef _STRING
}

/*
    UPALPHA        = <any US-ASCII uppercase letter "A".."Z">
    LOALPHA        = <any US-ASCII lowercase letter "a".."z">
    ALPHA          = UPALPHA | LOALPHA
 */
#define CASE_ALPHA    \
    case 'a' ... 'z': \
    case 'A' ... 'Z'

/*
    DIGIT          = <any US-ASCII digit "0".."9">
*/
#define CASE_DIGIT \
    case '0' ... '9'

/*
    scheme      = 1*( alpha | digit | "+" | "-" | "." )
*/
#define CASE_SCHEME \
    CASE_ALPHA:     \
    CASE_DIGIT:     \
    case '+':       \
    case '-':       \
    case '.'

/*
    unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
    rfc3986
*/
#define CASE_UNRESERVED \
    CASE_ALPHA:         \
    CASE_DIGIT:         \
    case '-':           \
    case '.':           \
    case '_':           \
    case '~'

/*
      gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
      rfc3986
*/
#define CASE_GEN_DELIMS \
    case ':':           \
    case '/':           \
    case '?':           \
    case '#':           \
    case '[':           \
    case ']':           \
    case '@'

/*
    sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
                / "*" / "+" / "," / ";" / "="
    rfc3986 
*/
#define CASE_SUB_DELIMS \
    case '!':           \
    case '$':           \
    case '&':           \
    case '\'':          \
    case '(':           \
    case ')':           \
    case '*':           \
    case '+':           \
    case ',':           \
    case ';':           \
    case '='

/*
    reserved    = gen-delims / sub-delims
    rfc3986
*/
#define CASE_RESERVED \
    CASE_GEN_DELIMS:  \
    CASE_SUB_DELIMS

/*
    hex         = digit | "A" | "B" | "C" | "D" | "E" | "F" |
                          "a" | "b" | "c" | "d" | "e" | "f"
    rfc2616
*/
#define CASE_HEX      \
    CASE_DIGIT:       \
    case 'a' ... 'f': \
    case 'A' ... 'F'

/*
    pct-encoded = "%" HEXDIG HEXDIG
    rfc3986
*/
#define CASE_PCT_ENCODED \
    case '%':            \
        CASE_HEX

/*
    pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
    rfc3986
*/
#define CASE_PCHAR                                                                        \
    CASE_UNRESERVED:                                                                      \
    /* UNRESERVED contains most of PCT-ENCODED (ALPHA and DIGIT), we just need the '%' */ \
    case '%':                                                                             \
    CASE_SUB_DELIMS:                                                                      \
    case '@':                                                                             \
    case ':'

/*
    CTL            = <any US-ASCII control character
                     (octets 0 - 31) and DEL (127)>
    rfc2616
*/
#define CASE_CTL   \
    case 0 ... 31: \
    case 127

/*
    CHAR           = <any US-ASCII character (octets 0 - 127)>
    rfc2616
*/
#define CASE_CHAR \
    case 0 ... 127

/*
    separators     = "(" | ")" | "<" | ">" | "@"
                    | "," | ";" | ":" | "\" | <">
                    | "/" | "[" | "]" | "?" | "="
                    | "{" | "}" | SP | HT
    rfc2616
*/
#define CASE_SEPARATORS_WITHOUT_TAB \
    case '(':                       \
    case ')':                       \
    case '@':                       \
    case ',':                       \
    case ';':                       \
    case ':':                       \
    case '\\':                      \
    case '<':                       \
    case '>':                       \
    case '/':                       \
    case '[':                       \
    case ']':                       \
    case '?':                       \
    case '=':                       \
    case '{':                       \
    case '}':                       \
    case ' '
#define CASE_SEPARATORS          \
    CASE_SEPARATORS_WITHOUT_TAB: \
    case '\t'

/*
    A URI is composed from a limited set of characters consisting of
    digits, letters, and a few graphic symbols.  A reserved subset of
    those characters may be used to delimit syntax components within a
    URI while the remaining characters, including both the unreserved set
    and those reserved characters not acting as delimiters, define each
    component's identifying data.
    rfc3986
*/
#define CASE_URI   \
    CASE_RESERVED: \
    CASE_UNRESERVED

/*
    token          = 1*<any CHAR except CTLs or separators>
    rfc2616
*/
int is_token(unsigned char c)
{
    switch (c)
    {
    CASE_CTL:
        /* tab is both SEPARATORS and CTL
          * which would violate the 'no duplicate case values' rule.
          */
    CASE_SEPARATORS_WITHOUT_TAB:
        return 0;
    default:
        return 1;
    }
}

int is_alpha(char c)
{
    switch (c)
    {
    CASE_ALPHA:
        return 1;
    default:
        return 0;
    }
}

/*
    Method         = "OPTIONS"                ; Section 9.2
                    | "GET"                    ; Section 9.3
                    | "HEAD"                   ; Section 9.4
                    | "POST"                   ; Section 9.5
                    | "PUT"                    ; Section 9.6
                    | "DELETE"                 ; Section 9.7
                    | "TRACE"                  ; Section 9.8
                    | "CONNECT"                ; Section 9.9
                    | extension-method
    extension-method = token
    rfc2616
*/
int is_method(char c)
{
    return is_alpha(c);
}

int is_char(char c)
{
    switch (c)
    {
    CASE_CHAR:
        return 1;
    default:
        return 0;
    }
}

int is_digit(char c)
{
    switch (c)
    {
    CASE_DIGIT:
        return 1;
    default:
        return 0;
    }
}

int is_scheme(char c)
{
    switch (c)
    {
    CASE_SCHEME:
        return 1;
    default:
        return 0;
    }
}

int is_unreserved(char c)
{
    switch (c)
    {
    CASE_UNRESERVED:
        return 1;
    default:
        return 0;
    }
}

int is_reserved(char c)
{
    switch (c)
    {
    CASE_RESERVED:
        return 1;
    default:
        return 0;
    }
}

int is_pchar(char c)
{
    switch (c)
    {
    CASE_PCHAR:
        return 1;
    default:
        return 0;
    }
}

int is_seperator(char c)
{
    switch (c)
    {
    CASE_SEPARATORS:
        return 1;
    default:
        return 0;
    }
}

int is_ctl(unsigned char c)
{
    switch (c)
    {
    CASE_CTL:
        return 1;
    default:
        return 0;
    }
}

int is_space(char c)
{
    switch (c)
    {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        return 1;
    default:
        return 0;
    }
}

int is_uri(char c)
{
    switch (c)
    {
    CASE_URI:
        return 1;
    default:
        return 0;
    }
}

/*
    field-name     = token
*/
int is_header_name(char c)
{
    return is_token(c);
}

// https://stackoverflow.com/a/48138818
int is_header_value(char c)
{
    return is_char(c) || !is_space(c);
}

/*
 *    Request        = Request-Line
 *                     *(( general-header | request-header | entity-header ) CRLF)
 *                     CRLF
 *                     [ message-body ]
 *    Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
 *    HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
 *    message-header = field-name ":" [ field-value ]

 *    

 *
 * example:
 *     GET /index.html HTTP/1.1
 *     Host: www.google.com
 *     Accept: content-type/application-json
 */
enum http_state http_state_next(char last, char c)
{
    switch (last)
    {

    case s_init:
    {
        if (is_method(c))
        {
            return s_method;
        }
        return s_error;
    }

    case s_method:
    {
        if (is_method(c))
        {
            return s_method;
        }
        if (is_space(c))
        {
            return s_method_sp;
        }
        return s_error;
    }

    case s_method_sp:
    {
        if (is_space(c))
        {
            return s_method_sp;
        }
        if (is_uri(c))
        {
            return s_uri;
        }
        return s_error;
    }

    case s_uri:
    {
        if (is_uri(c))
        {
            return s_uri;
        }
        if (is_space(c))
        {
            return s_uri_sp;
        }
        return s_error;
    }

    case s_uri_sp:
    {
        if (is_space(c))
        {
            return s_uri_sp;
        }
        if (c == 'H')
        {
            return s_h;
        }
        return s_error;
    }

    case s_h:
    {
        if (c == 'T')
        {
            return s_ht;
        }
        return s_error;
    }

    case s_ht:
    {
        if (c == 'T')
        {
            return s_htt;
        }
        return s_error;
    }

    case s_htt:
    {
        if (c == 'P')
        {
            return s_http;
        }
        return s_error;
    }

    case s_http:
    {
        if (c == '/')
        {
            return s_http_slash;
        }
        return s_error;
    }

    case s_http_slash:
    {
        if (c == '1')
        {
            return s_http_slash_1;
        }
        return s_error;
    }

    case s_http_slash_1:
    {
        if (c == '.')
        {
            return s_http_slash_1_dot;
        }
        return s_error;
    }

    case s_http_slash_1_dot:
    {
        if (c == '1' || c == '0')
        {
            return s_http_slash_1_dot_x;
        }
        return s_error;
    }

    case s_http_slash_1_dot_x:
    {
        if (c == '\r')
        {
            return s_status_cr;
        }
        if (c == '\n')
        {
            return s_status_lf;
        }
        return s_error;
    }

    case s_status_cr:
    {
        if (c == '\n')
        {
            return s_status_lf;
        }
        return s_error;
    }

    case s_status_lf:
    {
        if (is_header_name(c))
        {
            return s_header_name;
        }
        // edge case: what if no headers are sent? technically invalid but we accept it anyway.
        if (c == '\r')
        {
            return s_end_cr;
        }
        if (c == '\n')
        {
            return s_end_lf;
        }
        return s_error;
    }

    case s_header_name:
    {
        if (is_header_name(c))
        {
            return s_header_name;
        }
        if (c == ':')
        {
            return s_header_colon;
        }
        return s_error;
    }

    case s_header_colon:
    {
        if (is_space(c))
        {
            return s_header_sp;
        }
        if (is_header_value(c))
        {
            return s_header_value;
        }
        return s_error;
    }

    case s_header_sp:
    {
        if (is_space(c))
        {
            return s_header_sp;
        }
        if (is_header_value(c))
        {
            return s_header_value;
        }
        return s_error;
    }

    case s_header_value:
    {
        if (c == '\r')
        {
            return s_header_cr;
        }
        if (c == '\n')
        {
            return s_header_lf;
        }
        if (is_header_value(c))
        {
            return s_header_value;
        }
        return s_error;
    }

    case s_header_cr:
    {
        if (c == '\n')
        {
            return s_header_lf;
        }
        return s_error;
    }

    case s_header_lf:
    {
        if (is_header_name(c))
        {
            return s_header_name;
        }
        if (c == '\r')
        {
            return s_end_cr;
        }
        if (c == '\n')
        {
            return s_end_lf;
        }
        return s_error;
    }

    case s_end_cr:
    {
        if (c == '\n')
        {
            return s_end_lf;
        }
        return s_error;
    }

    case s_end_lf:
    {
        return s_error;
    }

    case s_error:
    {
        return s_error;
    }

    default:
        break;
    }

    return s_error;
}
