#ifndef CGI_LOGIN_AUTH_H
#define CGI_LOGIN_AUTH_H

#define AUTH_OK              0
#define AUTH_INVALID_PASS    1
#define AUTH_ERROR           2

#define MAX_PASSWORD_LEN     64
#define DEFAULT_PASSWORD     "12345678"

int auth_check_password(const char *password);
void auth_log_attempt(const char *ip, int result);

#endif
