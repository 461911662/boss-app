#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "router/router.h"

int main(int argc, char *argv[])
{
    cgi_request_t req;
    cgi_response_t resp = {0};

    cgi_request_init(&req, getenv("PATH_INFO"), getenv("QUERY_STRING"),
                    getenv("REQUEST_METHOD"), getenv("REMOTE_ADDR"));

    CGI_LOG("Request from %s, path=%s, method=%s",
            req.remote_ip,
            req.path_info ? req.path_info : "null",
            req.method ? req.method : "null");

    int ret = 0;

    if (!req.path_info)
    {
        cgi_response_send_error(&resp, "No path info");
    }
    else
    {
        ret = cgi_route_dispatch(&req, &resp);
    }

    cgi_request_free(&req);
    cgi_response_free(&resp);
    return ret;
}
