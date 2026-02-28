#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

#define POST_BUF_SIZE 512

static void parse_form_data(cJSON *body, char *data)
{
    if (!body || !data)
        return;

    char *p = data;
    while (*p)
    {
        char *key = p;
        char *value = strchr(p, '=');
        if (!value)
            break;

        *value = '\0';
        value++;

        char *end = strchr(value, '&');
        if (end)
            *end = '\0';

        cgi_url_decode(key, key, 256);
        cgi_url_decode(value, value, 256);

        cJSON_AddStringToObject(body, key, value);

        if (end)
            p = end + 1;
        else
            break;
    }
}

void cgi_request_init(cgi_request_t *req, const char *path_info,
                      const char *query_string, const char *method,
                      const char *remote_ip)
{
    memset(req, 0, sizeof(cgi_request_t));

    req->path_info = path_info;
    req->query_string = query_string;
    req->method = method;
    req->remote_ip = remote_ip ? remote_ip : "unknown";

    req->body = cJSON_CreateObject();

    if (query_string && strlen(query_string) > 0)
    {
        char buf[POST_BUF_SIZE];
        strncpy(buf, query_string, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        parse_form_data(req->body, buf);
    }

    if (method && strcmp(method, "POST") == 0)
    {
        const char *content_len_str = getenv("CONTENT_LENGTH");
        int content_length = content_len_str ? atoi(content_len_str) : 0;

        if (content_length > 0 && content_length < POST_BUF_SIZE)
        {
            char buf[POST_BUF_SIZE];
            int len = 0;
            int ch;
            while (len < content_length)
            {
                ch = getchar();
                if (ch == EOF)
                    break;
                buf[len++] = (char)ch;
            }
            buf[len] = '\0';
            parse_form_data(req->body, buf);
        }
    }
}

void cgi_request_free(cgi_request_t *req)
{
    if (req->body)
    {
        cJSON_Delete(req->body);
        req->body = NULL;
    }
}

void cgi_response_free(cgi_response_t *resp)
{
    if (!resp)
        return;

    if (resp->data)
    {
        cJSON_Delete(resp->data);
        resp->data = NULL;
    }

    if (resp->redirect)
    {
        free(resp->redirect);
        resp->redirect = NULL;
    }
}

cJSON *cgi_json_get(cgi_request_t *req, const char *key)
{
    if (!req || !req->body || !key)
        return NULL;
    return cJSON_GetObjectItem(req->body, key);
}

const char *cgi_json_get_string(cgi_request_t *req, const char *key, const char *default_val)
{
    cJSON *item = cgi_json_get(req, key);
    if (!item || item->type != cJSON_String)
        return default_val;
    return item->valuestring;
}

int cgi_json_get_int(cgi_request_t *req, const char *key, int default_val)
{
    cJSON *item = cgi_json_get(req, key);
    if (!item || item->type != cJSON_Number)
        return default_val;
    return item->valueint;
}

bool cgi_json_get_bool(cgi_request_t *req, const char *key, bool default_val)
{
    cJSON *item = cgi_json_get(req, key);
    if (!item)
        return default_val;
    if (item->type == cJSON_True)
        return true;
    if (item->type == cJSON_False)
        return false;
    return default_val;
}

char *cgi_url_decode(const char *src, char *dest, size_t destlen)
{
    if (!src || !dest || destlen == 0)
        return dest;

    size_t j = 0;
    for (size_t i = 0; src[i] && j < destlen - 1; i++)
    {
        if (src[i] == '%' && src[i + 1] && src[i + 2])
        {
            char hex[3] = { src[i + 1], src[i + 2], 0 };
            int val;
            if (sscanf(hex, "%x", &val) == 1)
            {
                dest[j++] = (char)val;
                i += 2;
            }
            else
            {
                dest[j++] = src[i];
            }
        }
        else if (src[i] == '+')
        {
            dest[j++] = ' ';
        }
        else
        {
            dest[j++] = src[i];
        }
    }

    dest[j] = '\0';
    return dest;
}

int cgi_response_send_json(cgi_response_t *resp)
{
    if (!resp)
        return -1;

    printf("Content-Type: application/json\r\n");
    printf("Access-Control-Allow-Origin: *\r\n");
    printf("\r\n");

    if (resp->data)
    {
        char *json_str = cJSON_PrintUnformatted(resp->data);
        if (json_str)
        {
            printf("%s\n", json_str);
            free(json_str);
        }
        cJSON_Delete(resp->data);
        resp->data = NULL;
    }

    return 0;
}

int cgi_response_send_redirect(cgi_response_t *resp)
{
    if (!resp || !resp->redirect)
        return -1;

    printf("Status: 302 Found\r\n");
    printf("Location: %s\r\n", resp->redirect);
    printf("Content-Type: text/html\r\n");
    printf("\r\n");
    printf("<html><body><p>Redirecting to <a href=\"%s\">%s</a></p></body></html>\n",
           resp->redirect, resp->redirect);

    return 0;
}

int cgi_response_send_error(cgi_response_t *resp, const char *msg)
{
    if (!resp)
        return -1;

    resp->status_code = 400;
    resp->data = cJSON_CreateObject();
    cJSON_AddBoolToObject(resp->data, "success", false);
    cJSON_AddStringToObject(resp->data, "error", msg ? msg : "Unknown error");

    return cgi_response_send_json(resp);
}
