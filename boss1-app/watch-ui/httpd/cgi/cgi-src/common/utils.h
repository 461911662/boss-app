#ifndef CGI_COMMON_UTILS_H
#define CGI_COMMON_UTILS_H

#include <stddef.h>

char *cgi_get_param(const char *data, const char *name, char *buf, size_t buflen);
char *cgi_url_decode(const char *src, char *dest, size_t destlen);
void cgi_json_response_start(void);
void cgi_json_response_end(void);
void cgi_json_error(const char *msg);
void cgi_json_success(const char *msg);
void cgi_html_redirect(const char *url);

#endif
