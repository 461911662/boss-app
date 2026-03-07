#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

#define POST_BUF_SIZE 512

static void trim_string(char *str)
{
    if (!str || !*str)
        return;

    while (*str == ' ')
        str++;

    char *end = str + strlen(str) - 1;
    while (end > str && *end == ' ')
        *end-- = '\0';
}

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

        trim_string(key);
        trim_string(value);

        cgi_url_decode(key, key, 256);
        cgi_url_decode(value, value, 256);

        cJSON_AddStringToObject(body, key, value);

        if (end)
            p = end + 1;
        else
            break;
    }
}

void cgi_request_init(cgi_request_t *req)
{
    memset(req, 0, sizeof(cgi_request_t));

    req->path_info = getenv("PATH_INFO");
    req->query_string = getenv("QUERY_STRING");
    req->method = getenv("REQUEST_METHOD");
    req->remote_ip = getenv("REMOTE_ADDR");
    req->content_length = atoi(getenv("CONTENT_LENGTH") ? getenv("CONTENT_LENGTH") : "0");
    req->content_type = cgi_get_content_type(getenv("CONTENT_TYPE"));

    req->body = cJSON_CreateObject();

    const char *protocol = getenv("SERVER_PROTOCOL");
    const char *content_type = getenv("CONTENT_TYPE");
    CGI_LOG("%s %s %s from %s", req->method ? req->method : "GET",
            req->path_info ? req->path_info : "/",
            protocol ? protocol : "HTTP/1.1", req->remote_ip ? req->remote_ip : "null");
    if (req->content_length > 0)
    {
        CGI_LOG("Content-Type: %s", content_type ? content_type : "");
    }

    const char *query_string = req->query_string;
    if (query_string && strlen(query_string) > 0)
    {
        char *buf = strndup(query_string, 256);
        if (buf)
        {
            parse_form_data(req->body, buf);
            free(buf);
        }
    }

    CGI_LOG("Body:");
    if (req->method && strcmp(req->method, "POST") == 0 && req->content_length > 0)
    {
        char *buf = malloc(req->content_length + 1);
        if (buf)
        {
            int len = 0;
            int ch;
            while (len < req->content_length)
            {
                ch = getchar();
                if (ch == EOF)
                    break;
                buf[len++] = (char)ch;
            }
            buf[len] = '\0';

            if (len != req->content_length)
            {
                CGI_LOG("WARN: content_length mismatch, expected %d, got %d", req->content_length, len);
            } else {
                if (req->content_type == CGI_CONTENT_TYPE_JSON)
                {
                    cJSON *json = cJSON_Parse(buf);
                    if (json)
                    {
                        cJSON_Delete(req->body);
                        req->body = json;
                    }
                }
                else if (req->content_type == CGI_CONTENT_TYPE_FORM_URLENCODED)
                {
                    parse_form_data(req->body, buf);
                }

                if (req->body)
                {
                    char *json_str = cJSON_Print(req->body);
                    if (json_str)
                    {
                        CGI_LOG("\t%s", json_str);
                        free(json_str);
                    }
                }
            }
            free(buf);
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
