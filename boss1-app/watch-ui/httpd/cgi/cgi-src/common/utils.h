#ifndef CGI_COMMON_UTILS_H
#define CGI_COMMON_UTILS_H

#include <stddef.h>
#include <stdbool.h>

#ifdef CONFIG_MY_HTTPD_CGI_USE_CJSON
#include <netutils/cJSON.h>
#endif

#ifdef CONFIG_MY_HTTPD_CGI_DEBUG
#define CGI_LOG(fmt, ...) fprintf(stderr, "[CGI] " fmt "\n", ##__VA_ARGS__)
#else
#define CGI_LOG(fmt, ...)
#endif

#ifdef CONFIG_MY_HTTPD_CGI_USE_CJSON
struct cgi_request { const char *path_info; const char *query_string; const char *method; const char *remote_ip; cJSON *body; };
struct cgi_response { int status_code; cJSON *data; char *redirect; };
#else
struct cgi_request { const char *path_info; const char *query_string; const char *method; const char *remote_ip; };
struct cgi_response { int status_code; char *data; char *redirect; };
#endif

typedef struct cgi_request cgi_request_t;
typedef struct cgi_response cgi_response_t;

typedef int (*cgi_handler_t)(cgi_request_t *req, cgi_response_t *resp);

void cgi_request_init(cgi_request_t *req, const char *path_info,
                      const char *query_string, const char *method,
                      const char *remote_ip);
void cgi_request_free(cgi_request_t *req);

void cgi_response_free(cgi_response_t *resp);

cJSON *cgi_json_get(cgi_request_t *req, const char *key);
const char *cgi_json_get_string(cgi_request_t *req, const char *key, const char *default_val);
int cgi_json_get_int(cgi_request_t *req, const char *key, int default_val);
bool cgi_json_get_bool(cgi_request_t *req, const char *key, bool default_val);

char *cgi_url_decode(const char *src, char *dest, size_t destlen);

int cgi_response_send_json(cgi_response_t *resp);
int cgi_response_send_redirect(cgi_response_t *resp);
int cgi_response_send_error(cgi_response_t *resp, const char *msg);

int handle_login(cgi_request_t *req, cgi_response_t *resp);
int handle_status(cgi_request_t *req, cgi_response_t *resp);

#endif
