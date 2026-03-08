#include <string.h>
#include <stdlib.h>
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

    const char *username = cgi_json_get_string(req, "username", "");
    if (!username || strlen(username) == 0)
    {
        return cgi_response_send_error(resp, "Username is required");
    }

    int auth_result = auth_verify_user(username, password);

    if (auth_result == AUTH_OK)
    {
        auth_log_attempt(req->remote_ip, AUTH_OK);

        resp->data = cJSON_CreateObject();
        cJSON_AddBoolToObject(resp->data, "success", true);
        cJSON_AddStringToObject(resp->data, "message", "Login successful");
        resp->redirect = strdup("/content.html");

        cgi_response_send_json(resp);
        return cgi_response_send_redirect(resp);
    }
    else
    {
        auth_log_attempt(req->remote_ip, auth_result);
        
        const char *error_msg;
        switch (auth_result)
        {
            case AUTH_INVALID_PASS:
                error_msg = "Incorrect password";
                break;
            case AUTH_ERROR:
                error_msg = "Invalid auth password";
                break;
            case AUTH_USER_EXCEED:
                error_msg = "Maximum number of users reached";
                break;
            case AUTH_INNER_ERROR:
            default:
                error_msg = "Internal error";
                break;
        }
        
        return cgi_response_send_error(resp, error_msg);
    }
}
