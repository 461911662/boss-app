#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "login/auth.h"
#include "login/session.h"
#include "common/utils.h"
#include "status/wifi_status.h"

#define CGI_LOG(fmt, ...) fprintf(stderr, "[CGI] " fmt "\n", ##__VA_ARGS__)

#define POST_BUF_SIZE 512

int login_main_logic(const char *remote_ip);

int main(int argc, char *argv[])
{
    const char *path = getenv("SCRIPT_NAME");
    const char *remote_ip = getenv("REMOTE_ADDR");

    if (!remote_ip)
        remote_ip = "unknown";

    CGI_LOG("Request from %s, path=%s", remote_ip, path ? path : "null");

    if (!path)
    {
        cgi_json_error("No script path");
        return 0;
    }

    if (strstr(path, "status") != NULL)
    {
        wifi_status_t status;
        wifi_device_t devices[MAX_DEVICES];
        int device_count;

        wifi_get_status(&status);
        device_count = wifi_get_connected_devices(devices, MAX_DEVICES);

        cgi_json_response_start();

        printf("{");
        printf("\"connected\": %s,", status.connected ? "true" : "false");
        printf("\"ssid\": \"%s\",", status.ssid);
        printf("\"rssi\": %d,", status.rssi);
        printf("\"channel\": %d,", status.channel);
        printf("\"band\": \"%s\",", status.band);
        printf("\"ip\": \"%s\",", status.ip);
        printf("\"devices\": [");

        for (int i = 0; i < device_count; i++)
        {
            if (i > 0)
                printf(",");
            printf("{\"name\": \"%s\", \"ip\": \"%s\"}", devices[i].name, devices[i].ip);
        }

        printf("]");
        printf("}\n");

        return 0;
    }

    if (strstr(path, "login") != NULL)
    {
        return login_main_logic(remote_ip);
    }

    cgi_json_error("Unknown CGI command");
    return 0;
}

int login_main_logic(const char *remote_ip)
{
    session_init();

    const char *method = getenv("REQUEST_METHOD");
    if (!method)
    {
        cgi_json_error("No request method");
        return 0;
    }

    if (strcmp(method, "POST") != 0)
    {
        cgi_json_error("Invalid request method");
        return 0;
    }

    char *content_length_str = getenv("CONTENT_LENGTH");
    int content_length = 0;

    if (content_length_str)
    {
        content_length = atoi(content_length_str);
    }

    if (content_length <= 0 || content_length >= POST_BUF_SIZE)
    {
        content_length = POST_BUF_SIZE - 1;
    }

    char post_data[POST_BUF_SIZE];
    memset(post_data, 0, sizeof(post_data));

    int read_len = 0;
    int ch;
    while (read_len < content_length)
    {
        ch = getchar();
        if (ch == EOF)
            break;
        post_data[read_len++] = (char)ch;
    }
    post_data[read_len] = '\0';

    char password[128];
    memset(password, 0, sizeof(password));

    cgi_get_param(post_data, "password", password, sizeof(password));

    if (strlen(password) == 0)
    {
        cgi_json_error("Password is required");
        return 0;
    }

    int auth_result = auth_check_password(password);

    if (auth_result == AUTH_OK)
    {
        session_add(remote_ip);
        auth_log_attempt(remote_ip, AUTH_OK);
        cgi_html_redirect("/success.html");
    }
    else
    {
        auth_log_attempt(remote_ip, AUTH_INVALID_PASS);
        cgi_json_error("Invalid password");
    }

    return 0;
}
