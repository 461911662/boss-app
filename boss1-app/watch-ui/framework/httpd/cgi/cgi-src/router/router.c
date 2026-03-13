#include <string.h>
#include "router.h"

#include "common/utils.h"
#include "login/handle_login.h"
#include "status/handle_status.h"

static cgi_route_t g_cgi_routes[] = {
    { "/login",  handle_login },
    { "/status", handle_status },
    { NULL, NULL }
};

int cgi_route_dispatch(cgi_request_t *req, cgi_response_t *resp)
{
    const char *path = req->path_info;

    CGI_LOG("Dispatch: path=%s", path ? path : "null");

    for (int i = 0; g_cgi_routes[i].path != NULL; i++)
    {
        if (strcmp(path, g_cgi_routes[i].path) == 0)
        {
            CGI_LOG("Route matched: %s", path);
            return g_cgi_routes[i].handler(req, resp);
        }
    }

    CGI_LOG("Route not found: %s", path);
    resp->status_code = 404;
    resp->data = cJSON_CreateObject();
    cJSON_AddBoolToObject(resp->data, "success", false);
    cJSON_AddStringToObject(resp->data, "error", "Not found");

    return cgi_response_send_json(resp);
}
