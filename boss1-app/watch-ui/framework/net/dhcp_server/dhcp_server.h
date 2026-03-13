/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/dhcp_server/dhcp_server.h
 *
 * DHCP Server API using libuv
 ****************************************************************************/

#ifndef __DHCP_SERVER_H__
#define __DHCP_SERVER_H__


/* Global defines -----------------------------------------------------------*/
/* Global enums -------------------------------------------------------------*/
/* Global structs -----------------------------------------------------------*/
/* Global typedefs ----------------------------------------------------------*/
/* Global enums -------------------------------------------------------------*/
/* Global unions ------------------------------------------------------------*/
/* Global variables ---------------------------------------------------------*/
/* Global variables prototypes ----------------------------------------------*/
/* Global functions ---------------------------------------------------------*/


/* Includes -----------------------------------------------------------------*/
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

/* Global functions prototypes ----------------------------------------------*/
/**
 * @brief Start DHCP server for captive portal
 * 
 * @param loop      libuv event loop
 * @param interface Network interface name (e.g., "wlan1")
 * @param server_ip DHCP server IP address
 * @param start_ip DHCP pool start IP
 * @param end_ip DHCP pool end IP
 * @param netmask Network netmask
 * @return 0 on success, negative errno on failure
 */
int dhcp_server_start(uv_loop_t *loop, const char *interface,
                      in_addr_t server_ip, in_addr_t start_ip,
                      in_addr_t end_ip, in_addr_t netmask);

/**
 * @brief Stop DHCP server
 */
void dhcp_server_stop(void);

/**
 * @brief Check if DHCP server is running
 * 
 * @return true if running, false otherwise
 */
bool dhcp_server_is_running(void);

#ifdef __cplusplus
}
#endif

#endif /* __DHCP_SERVER_H__ */
