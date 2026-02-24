/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/httpd/httpd_server.h
 ****************************************************************************/

#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start HTTPD server
 * 
 * @return 0 on success, negative errno on failure
 */
int httpd_server_start(void);

/**
 * @brief Stop HTTPD server
 */
void httpd_server_stop(void);

/**
 * @brief Check if HTTP server is running
 * 
 * @return true if running, false otherwise
 */
bool httpd_server_is_running(void);

#ifdef __cplusplus
}
#endif

#endif /* __HTTPD_SERVER_H__ */
