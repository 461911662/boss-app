#include <stdio.h>
#include <stdlib.h>

#include "common/utils.h"
#include "router/router.h"

int main(int argc, char *argv[])
{
    cgi_request_t req = {0};
    cgi_response_t resp = {0};

    cgi_request_init(&req);

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
