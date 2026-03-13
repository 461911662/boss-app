#ifndef CGI_ROUTER_H
#define CGI_ROUTER_H

#include "common/utils.h"

typedef struct
{
    const char *path;
    cgi_handler_t handler;
} cgi_route_t;

int cgi_route_dispatch(cgi_request_t *req, cgi_response_t *resp);

#endif
