#include <stdio.h>
#include <string.h>
#include <time.h>
#include "session.h"

static session_client_t g_clients[MAX_CLIENTS];
static int g_client_count = 0;

void session_init(void)
{
    memset(g_clients, 0, sizeof(g_clients));
    g_client_count = 0;
}

session_client_t *session_find(const char *ip)
{
    if (!ip)
        return NULL;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (g_clients[i].authenticated && strcmp(g_clients[i].ip, ip) == 0)
        {
            time_t now = time(NULL);
            if (now - g_clients[i].login_time < SESSION_TIMEOUT)
            {
                return &g_clients[i];
            }
            else
            {
                g_clients[i].authenticated = 0;
            }
        }
    }

    return NULL;
}

session_client_t *session_add(const char *ip)
{
    if (!ip)
        return NULL;

    session_client_t *client = session_find(ip);
    if (client)
    {
        client->login_time = time(NULL);
        return client;
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!g_clients[i].authenticated)
        {
            strncpy(g_clients[i].ip, ip, sizeof(g_clients[i].ip) - 1);
            g_clients[i].ip[sizeof(g_clients[i].ip) - 1] = '\0';
            g_clients[i].login_time = time(NULL);
            g_clients[i].authenticated = 1;
            return &g_clients[i];
        }
    }

    return NULL;
}

void session_remove(const char *ip)
{
    if (!ip)
        return;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (g_clients[i].authenticated && strcmp(g_clients[i].ip, ip) == 0)
        {
            g_clients[i].authenticated = 0;
            break;
        }
    }
}

void session_cleanup(void)
{
    time_t now = time(NULL);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (g_clients[i].authenticated)
        {
            if (now - g_clients[i].login_time >= SESSION_TIMEOUT)
            {
                g_clients[i].authenticated = 0;
            }
        }
    }
}
