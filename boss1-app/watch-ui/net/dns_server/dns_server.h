/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/dns_server/dns_server.h
 *
 * Captive Portal DNS Server API
 ****************************************************************************/

#ifndef __DNS_SERVER_H__
#define __DNS_SERVER_H__


/* Global defines -----------------------------------------------------------*/
/* Global enums -------------------------------------------------------------*/
/* Global typedefs ----------------------------------------------------------*/
/* Global structs -----------------------------------------------------------*/
/* Global union -------------------------------------------------------------*/
/* Global variables ---------------------------------------------------------*/
/* Global variables prototypes ----------------------------------------------*/

/* Include ------------------------------------------------------------------*/
#include <nuttx/config.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef CONFIG_LIBUV
#include <uv.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Global function prototypes -----------------------------------------------*/

/**
 * @brief Start DNS server for captive portal
 * 
 * @param loop      libuv event loop
 * @param portal_ip The IP address to return for all DNS queries (AP IP)
 * @return 0 on success, negative errno on failure
 */
int dns_server_start(uv_loop_t *loop, in_addr_t portal_ip);

/**
 * @brief Stop DNS server
 */
void dns_server_stop(void);

/**
 * @brief Check if DNS server is running
 * 
 * @return true if running, false otherwise
 */
bool dns_server_is_running(void);

#ifdef __cplusplus
}
#endif

#endif /* __DNS_SERVER_H__ */
