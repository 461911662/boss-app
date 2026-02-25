#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

char *cgi_get_param(const char *data, const char *name, char *buf, size_t buflen)
{
    if (!data || !name || !buf || buflen == 0)
        return NULL;

    char search[64];
    snprintf(search, sizeof(search), "%s=", name);

    char *p = strstr(data, search);
    if (!p)
        return NULL;

    p += strlen(search);

    char *end = strchr(p, '&');
    size_t len = end ? (size_t)(end - p) : strlen(p);

    if (len >= buflen)
        len = buflen - 1;

    memcpy(buf, p, len);
    buf[len] = '\0';

    cgi_url_decode(buf, buf, buflen);

    return buf;
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

void cgi_json_response_start(void)
{
    printf("Content-type: application/json\r\n");
    printf("Cache-Control: no-cache\r\n");
    printf("\r\n");
}

void cgi_json_response_end(void)
{
}

void cgi_json_error(const char *msg)
{
    cgi_json_response_start();
    printf("{\"success\": false, \"message\": \"");
    while (*msg)
    {
        if (*msg == '"' || *msg == '\\')
            putchar('\\');
        putchar(*msg++);
    }
    printf("\"}\n");
}

void cgi_json_success(const char *msg)
{
    cgi_json_response_start();
    printf("{\"success\": true, \"message\": \"");
    while (*msg)
    {
        if (*msg == '"' || *msg == '\\')
            putchar('\\');
        putchar(*msg++);
    }
    printf("\"}\n");
}

void cgi_html_redirect(const char *url)
{
    printf("Status: 302 Found\r\n");
    printf("Location: %s\r\n", url);
    printf("Content-type: text/html\r\n");
    printf("\r\n");
    printf("<html><body><p>Redirecting to <a href=\"%s\">%s</a></p></body></html>\n", url, url);
}
