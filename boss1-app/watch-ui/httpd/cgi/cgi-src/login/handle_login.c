#include <string.h>
#include "utils.h"
#include "auth.h"

int handle_login(cgi_request_t *req, cgi_response_t *resp)
{
    const char *method = req->method;
    if (!method || strcmp(method, "POST") != 0)
    {
        return cgi_response_send_error(resp, "Invalid request method");
    }

    const char *password = cgi_json_get_string(req, "password", "");
    if (!password || strlen(password) == 0)
    {
        return cgi_response_send_error(resp, "Password is required");
    }

    int auth_result = auth_check_password(password);

    if (auth_result == AUTH_OK)
    {
        auth_log_attempt(req->remote_ip, AUTH_OK);

        resp->data = cJSON_CreateObject();
        cJSON_AddBoolToObject(resp->data, "success", true);
        cJSON_AddStringToObject(resp->data, "message", "Login successful");
        resp->redirect = "/success.html";

        cgi_response_send_json(resp);
        return cgi_response_send_redirect(resp);
    }
    else
    {
        auth_log_attempt(req->remote_ip, AUTH_INVALID_PASS);
        return cgi_response_send_error(resp, "Invalid password");
    }
}
