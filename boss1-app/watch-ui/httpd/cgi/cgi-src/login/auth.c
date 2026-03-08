#include <stdio.h>
#include <string.h>
#include <time.h>
#include <nuttx/config.h>
#include "auth.h"
#include "utils.h"

#define WIFI_PORTAL_USE_PASSWD 1
#define WIFI_PORTAL_USERNAME "author"
#define WIFI_PORTAL_USER_MAX 7

#ifdef WIFI_PORTAL_USE_PASSWD
#include <fsutils/passwd.h>
#endif

int auth_verify_user(const char *username, const char *password)
{
    if (!username || !password)
        return AUTH_INVALID_PASS;

#ifdef WIFI_PORTAL_USE_PASSWD
    if (passwd_finduser(username))
    {
        int ret = passwd_verify(username, password);
        if (ret == 1)
            return AUTH_OK;
        else if (ret == 0)
            return AUTH_INVALID_PASS;
        else
            return AUTH_INNER_ERROR;
    }
    else
    {
        int ret = passwd_verify(WIFI_PORTAL_USERNAME, password);
        if (ret != 1)
            return AUTH_ERROR;

        int user_count = passwd_get_user_count();
        if (user_count < 0)
            return AUTH_INNER_ERROR;

        if (user_count >= WIFI_PORTAL_USER_MAX)
            return AUTH_USER_EXCEED;

        ret = passwd_adduser(username, password);
        if (ret == 0)
            return AUTH_OK;
        else
            return AUTH_INNER_ERROR;
    }
#else
    return AUTH_OK;
#endif
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
        CGI_LOG("[%s] Login SUCCESS from %s\n", time_str ? time_str : "?", ip);
    }
    else
    {
        CGI_LOG("[%s] Login FAILED from %s\n", time_str ? time_str : "?", ip);
    }
}
