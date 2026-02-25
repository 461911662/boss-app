#ifndef CGI_LOGIN_SESSION_H
#define CGI_LOGIN_SESSION_H

#include <time.h>
#include <stddef.h>

#define MAX_CLIENTS 16
#define SESSION_TIMEOUT 3600

typedef struct
{
    char ip[32];
    time_t login_time;
    int authenticated;
} session_client_t;

void session_init(void);
session_client_t *session_find(const char *ip);
session_client_t *session_add(const char *ip);
void session_remove(const char *ip);
void session_cleanup(void);

#endif
