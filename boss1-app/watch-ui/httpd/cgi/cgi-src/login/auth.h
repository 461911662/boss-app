#ifndef CGI_LOGIN_AUTH_H
#define CGI_LOGIN_AUTH_H

#define AUTH_OK                  0   /* 校验成功 */
#define AUTH_INVALID_PASS        1   /* 用户密码失败 */
#define AUTH_USER_EXCEED         2   /* 超过最大用户数量 */
#define AUTH_ERROR               3   /* 用户录入的认证密码与认证账号不匹配（外部错误） */
#define AUTH_INNER_ERROR         4   /* 内部错误 */

#define MAX_PASSWORD_LEN     64
#define DEFAULT_PASSWORD     "12345678"

int auth_verify_user(const char *username, const char *password);
void auth_log_attempt(const char *ip, int result);

#endif
