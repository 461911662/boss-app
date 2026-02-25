#include <stdio.h>
#include <string.h>
#include <time.h>
#include "auth.h"

static const char *g_password = DEFAULT_PASSWORD;

int auth_check_password(const char *password)
{
    if (!password)
        return AUTH_INVALID_PASS;

    if (strcmp(password, g_password) == 0)
        return AUTH_OK;

    return AUTH_INVALID_PASS;
}

void auth_log_attempt(const char *ip, int result)
{
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    if (time_str)
    {
        time_str[strlen(time_str) - 1] = '\0';
    }

    if (result == AUTH_OK)
    {
        fprintf(stderr, "[%s] Login SUCCESS from %s\n", time_str ? time_str : "?", ip);
    }
    else
    {
        fprintf(stderr, "[%s] Login FAILED from %s\n", time_str ? time_str : "?", ip);
    }
}
