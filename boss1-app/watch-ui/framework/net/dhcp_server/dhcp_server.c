/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/dhcp_server/dhcp_server.c
 *
 * DHCP Server for NuttX using libuv
 * Full implementation with DHCPNAK, IP lease, MAC binding, renew support
 ****************************************************************************/

/* Private enums ------------------------------------------------------------*/
/* Private structs ----------------------------------------------------------*/
/* Private unions -----------------------------------------------------------*/
/* Global variables ---------------------------------------------------------*/
/* Private function prototypes ----------------------------------------------*/
/* Global function prototypes -----------------------------------------------*/

/* Includes -----------------------------------------------------------------*/
#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#include "dhcp_server.h"
#include "watchui/log.h"

/* Private defines ----------------------------------------------------------*/
/* Check configuration.  This is not all of the configuration settings that
 * are required -- only the more obvious.
 */
#ifndef CONFIG_LIBUV
#    error "You must select CONFIG_LIBUV in your configuration file"
#endif

/* DHCP Protocol Constants */
#define DHCP_SERVER_PORT     67
#define DHCP_CLIENT_PORT    68
#define DHCP_MAX_LEN        548
#define DHCP_MAX_LEASES     64
#define DHCP_DEFAULT_LEASE  3600    /* 1 hour in seconds */

/* DHCP Message Types */
#define DHCP_MSG_DISCOVER   1
#define DHCP_MSG_OFFER      2
#define DHCP_MSG_REQUEST    3
#define DHCP_MSG_DECLINE    4
#define DHCP_MSG_ACK        5
#define DHCP_MSG_NAK        6
#define DHCP_MSG_RELEASE    7
#define DHCP_MSG_INFORM     8

/* DHCP Options */
#define DHCP_OPT_PAD        0
#define DHCP_OPT_MSG_TYPE   53
#define DHCP_OPT_SERVER_ID  54
#define DHCP_OPT_LEASE_TIME 51
#define DHCP_OPT_REQ_IP     50
#define DHCP_OPT_SUBNET     1
#define DHCP_OPT_ROUTER     3
#define DHCP_OPT_DNS        6
#define DHCP_OPT_END        255

/* Logging macros */
#define dpinfo(fmt, ...)    appinfo("[DHCP] " fmt, ##__VA_ARGS__)
#define dpdbg(fmt, ...)     appdbg("[DHCP] " fmt, ##__VA_ARGS__)
#define dpwarn(fmt, ...)    appwarn("[DHCP] " fmt, ##__VA_ARGS__)
#define dperr(fmt, ...)     apperr("[DHCP] " fmt, ##__VA_ARGS__)

/* Private typedefs ---------------------------------------------------------*/
/* DHCP Header Structure */
typedef struct __attribute__((packed)) {
    uint8_t  op;
    uint8_t  htype;
    uint8_t  hlen;
    uint8_t  hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint32_t ciaddr;
    uint32_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;
    uint8_t  chaddr[16];
    uint8_t  sname[64];
    uint8_t  file[128];
    uint32_t magic;
} dhcp_header_t;

/* DHCP Lease Entry */
typedef struct {
    uint8_t  used;
    uint8_t  mac[6];
    uint32_t ip;
    uint32_t lease_start;
    uint32_t lease_time;
    uint32_t transaction_id;
} dhcp_lease_t;

/* DHCP Server Context */
typedef struct {
    uv_udp_t udp_handle;
    in_addr_t server_ip;
    in_addr_t start_ip;
    in_addr_t end_ip;
    in_addr_t netmask;
    in_addr_t router_ip;
    in_addr_t dns_ip;
    bool running;
    uint32_t lease_time;
    dhcp_lease_t leases[DHCP_MAX_LEASES];
} dhcp_server_ctx_t;

/* Private variables --------------------------------------------------------*/
static dhcp_server_ctx_t g_dhcp_ctx = {
    .running = false,
    .lease_time = DHCP_DEFAULT_LEASE
};

/* Private functions --------------------------------------------------------*/
/****************************************************************************
 * Get current time
 ****************************************************************************/
static uint32_t dhcp_get_time(void)
{
    return time(NULL);
}

/****************************************************************************
 * Find lease by MAC address
 ****************************************************************************/
static dhcp_lease_t* dhcp_find_lease_by_mac(const uint8_t *mac)
{
    for (int i = 0; i < DHCP_MAX_LEASES; i++) {
        if (g_dhcp_ctx.leases[i].used && 
            memcmp(g_dhcp_ctx.leases[i].mac, mac, 6) == 0) {
            return &g_dhcp_ctx.leases[i];
        }
    }
    return NULL;
}

/****************************************************************************
 * Find lease by IP address
 ****************************************************************************/
static dhcp_lease_t* dhcp_find_lease_by_ip(uint32_t ip)
{
    for (int i = 0; i < DHCP_MAX_LEASES; i++) {
        if (g_dhcp_ctx.leases[i].used && 
            g_dhcp_ctx.leases[i].ip == ip) {
            return &g_dhcp_ctx.leases[i];
        }
    }
    return NULL;
}

/****************************************************************************
 * Allocate a free lease entry
 ****************************************************************************/
static dhcp_lease_t* dhcp_alloc_lease(void)
{
    /* First, check for expired leases */
    uint32_t now = dhcp_get_time();
    for (int i = 0; i < DHCP_MAX_LEASES; i++) {
        if (g_dhcp_ctx.leases[i].used) {
            uint32_t elapsed = now - g_dhcp_ctx.leases[i].lease_start;
            if (elapsed > g_dhcp_ctx.leases[i].lease_time) {
                memset(&g_dhcp_ctx.leases[i], 0, sizeof(dhcp_lease_t));
                return &g_dhcp_ctx.leases[i];
            }
        }
    }
    
    /* Find a free slot */
    for (int i = 0; i < DHCP_MAX_LEASES; i++) {
        if (!g_dhcp_ctx.leases[i].used) {
            return &g_dhcp_ctx.leases[i];
        }
    }
    
    return NULL;
}

/****************************************************************************
 * Release a lease
 ****************************************************************************/
static void dhcp_release_lease(dhcp_lease_t *lease)
{
    if (lease) {
        memset(lease, 0, sizeof(dhcp_lease_t));
    }
}

/****************************************************************************
 * Allocate an IP address from the pool
 ****************************************************************************/
static uint32_t dhcp_alloc_ip(const uint8_t *mac, uint32_t request_ip, uint32_t xid)
{
    uint32_t start = ntohl(g_dhcp_ctx.start_ip);
    uint32_t end = ntohl(g_dhcp_ctx.end_ip);
    
    /* First, check if MAC already has a binding */
    dhcp_lease_t *existing = dhcp_find_lease_by_mac(mac);
    if (existing) {
        uint32_t now = dhcp_get_time();
        uint32_t elapsed = now - existing->lease_start;
        if (elapsed < existing->lease_time) {
            return existing->ip;
        }
    }
    
    /* If client requested a specific IP */
    if (request_ip != 0) {
        uint32_t req_ip_n = ntohl(request_ip);
        if (req_ip_n >= start && req_ip_n <= end) {
            dhcp_lease_t *ip_lease = dhcp_find_lease_by_ip(request_ip);
            if (ip_lease == NULL || ip_lease == existing) {
                if (existing) {
                    existing->ip = request_ip;
                    existing->lease_start = dhcp_get_time();
                    existing->lease_time = g_dhcp_ctx.lease_time;
                    existing->transaction_id = xid;
                }
                return request_ip;
            }
        }
    }
    
    /* Allocate new IP from pool */
    for (uint32_t current = start; current <= end; current++) {
        uint32_t ip = htonl(current);
        
        dhcp_lease_t *lease = dhcp_find_lease_by_ip(ip);
        if (lease != NULL && lease != existing) {
            continue;
        }
        
        if (existing) {
            existing->ip = ip;
            existing->lease_start = dhcp_get_time();
            existing->lease_time = g_dhcp_ctx.lease_time;
            existing->transaction_id = xid;
        } else {
            dhcp_lease_t *new_lease = dhcp_alloc_lease();
            if (new_lease) {
                new_lease->used = 1;
                memcpy(new_lease->mac, mac, 6);
                new_lease->ip = ip;
                new_lease->lease_start = dhcp_get_time();
                new_lease->lease_time = g_dhcp_ctx.lease_time;
                new_lease->transaction_id = xid;
            }
        }
        return ip;
    }
    
    return 0;
}

/****************************************************************************
 * Build DHCP response options
 ****************************************************************************/
static int dhcp_build_options(uint8_t *options, uint8_t msg_type, uint32_t server_id,
                             uint32_t lease_time, uint32_t subnet, uint32_t router)
{
    int opt_idx = 0;
    
    uint32_t magic = htonl(0x63825363);
    memcpy(options + opt_idx, &magic, 4);
    opt_idx += 4;
    
    options[opt_idx++] = DHCP_OPT_MSG_TYPE;
    options[opt_idx++] = 1;
    options[opt_idx++] = msg_type;
    
    options[opt_idx++] = DHCP_OPT_SERVER_ID;
    options[opt_idx++] = 4;
    memcpy(options + opt_idx, &server_id, 4);
    opt_idx += 4;
    
    options[opt_idx++] = DHCP_OPT_LEASE_TIME;
    options[opt_idx++] = 4;
    uint32_t lease = htonl(lease_time);
    memcpy(options + opt_idx, &lease, 4);
    opt_idx += 4;
    
    options[opt_idx++] = DHCP_OPT_SUBNET;
    options[opt_idx++] = 4;
    memcpy(options + opt_idx, &subnet, 4);
    opt_idx += 4;
    
    options[opt_idx++] = DHCP_OPT_ROUTER;
    options[opt_idx++] = 4;
    memcpy(options + opt_idx, &router, 4);
    opt_idx += 4;
    
    if (g_dhcp_ctx.dns_ip != 0) {
        options[opt_idx++] = DHCP_OPT_DNS;
        options[opt_idx++] = 4;
        memcpy(options + opt_idx, &g_dhcp_ctx.dns_ip, 4);
        opt_idx += 4;
    }
    
    options[opt_idx++] = DHCP_OPT_END;
    
    return opt_idx;
}

/****************************************************************************
 * Process DHCP Request
 ****************************************************************************/
static void dhcp_process_request(const uint8_t *req_buf, ssize_t req_len,
                                uint8_t *resp_buf, ssize_t *resp_len,
                                struct sockaddr_in *client_addr)
{
    (void)client_addr;
    
    if (req_len < (ssize_t)sizeof(dhcp_header_t)) {
        dperr("Request too short: %zd bytes", req_len);
        return;
    }
    
    const dhcp_header_t *req_hdr = (const dhcp_header_t *)req_buf;
    uint8_t msg_type = 0;
    uint32_t requested_ip = 0;
    uint8_t *options = (uint8_t *)req_buf + sizeof(dhcp_header_t);
    ssize_t options_len = req_len - sizeof(dhcp_header_t);
    
    for (ssize_t i = 0; i < options_len - 1; ) {
        uint8_t code = options[i];
        if (code == DHCP_OPT_END) break;
        if (code == DHCP_OPT_PAD) { i++; continue; }
        if (i + 1 >= options_len) break;
        uint8_t len = options[i + 1];
        if (i + 2 + len > options_len) break;
        
        if (code == DHCP_OPT_MSG_TYPE && len == 1) {
            msg_type = options[i + 2];
        } else if (code == DHCP_OPT_REQ_IP && len == 4) {
            memcpy(&requested_ip, options + i + 2, 4);
        }
        
        i += 2 + len;
    }
    
    dpinfo("DHCP msg=%d, xid=%u, mac=%02x:%02x:%02x:%02x:%02x:%02x",
              msg_type, req_hdr->xid,
              req_hdr->chaddr[0], req_hdr->chaddr[1], req_hdr->chaddr[2],
              req_hdr->chaddr[3], req_hdr->chaddr[4], req_hdr->chaddr[5]);
    
    dhcp_header_t *resp_hdr = (dhcp_header_t *)resp_buf;
    memcpy(resp_hdr, req_hdr, sizeof(dhcp_header_t));
    resp_hdr->op = 2;
    resp_hdr->siaddr = g_dhcp_ctx.server_ip;
    
    uint8_t resp_msg_type = 0;
    uint32_t assigned_ip = 0;
    int nak_reason = 0;
    
    switch (msg_type) {
        case DHCP_MSG_DISCOVER:
            assigned_ip = dhcp_alloc_ip(req_hdr->chaddr, 0, req_hdr->xid);
            if (assigned_ip == 0) {
                dperr("No available IP addresses");
                return;
            }
            resp_hdr->yiaddr = assigned_ip;
            resp_msg_type = DHCP_MSG_OFFER;
            break;
            
        case DHCP_MSG_REQUEST: {
            dhcp_lease_t *existing = dhcp_find_lease_by_mac(req_hdr->chaddr);
            
            if (requested_ip != 0) {
                uint32_t req_ip_n = ntohl(requested_ip);
                uint32_t start_n = ntohl(g_dhcp_ctx.start_ip);
                uint32_t end_n = ntohl(g_dhcp_ctx.end_ip);
                
                if (req_ip_n >= start_n && req_ip_n <= end_n) {
                    dhcp_lease_t *ip_lease = dhcp_find_lease_by_ip(requested_ip);
                    if (ip_lease == NULL || ip_lease == existing) {
                        assigned_ip = requested_ip;
                    } else {
                        nak_reason = 1;
                    }
                } else {
                    nak_reason = 1;
                }
            } else if (existing) {
                assigned_ip = existing->ip;
            } else {
                assigned_ip = dhcp_alloc_ip(req_hdr->chaddr, 0, req_hdr->xid);
            }
            
            if (nak_reason) {
                resp_msg_type = DHCP_MSG_NAK;
                dpinfo("Sending DHCPNAK, reason=%d", nak_reason);
            } else if (assigned_ip == 0) {
                dperr("No available IP addresses");
                return;
            } else {
                resp_hdr->yiaddr = assigned_ip;
                resp_msg_type = DHCP_MSG_ACK;
                
                if (existing) {
                    existing->lease_start = dhcp_get_time();
                    existing->lease_time = g_dhcp_ctx.lease_time;
                } else {
                    dhcp_lease_t *new_lease = dhcp_alloc_lease();
                    if (new_lease) {
                        new_lease->used = 1;
                        memcpy(new_lease->mac, req_hdr->chaddr, 6);
                        new_lease->ip = assigned_ip;
                        new_lease->lease_start = dhcp_get_time();
                        new_lease->lease_time = g_dhcp_ctx.lease_time;
                    }
                }
            }
            break;
        }
        
        case DHCP_MSG_DECLINE:
            if (req_hdr->yiaddr != 0) {
                dhcp_lease_t *lease = dhcp_find_lease_by_ip(req_hdr->yiaddr);
                if (lease) {
                    dpinfo("Client declined IP, releasing");
                    dhcp_release_lease(lease);
                }
            }
            return;
            
        case DHCP_MSG_RELEASE: {
            dhcp_lease_t *lease = dhcp_find_lease_by_mac(req_hdr->chaddr);
            if (lease) {
                dpinfo("Client released IP");
                dhcp_release_lease(lease);
            }
            return;
        }
        
        default:
            dpinfo("Unsupported DHCP message type: %d", msg_type);
            return;
    }
    
    uint8_t *resp_options = resp_buf + sizeof(dhcp_header_t);
    int opt_len = dhcp_build_options(resp_options, resp_msg_type,
                                     g_dhcp_ctx.server_ip, g_dhcp_ctx.lease_time,
                                     g_dhcp_ctx.netmask, g_dhcp_ctx.router_ip);
    
    *resp_len = sizeof(dhcp_header_t) + opt_len;
    
    if (resp_msg_type == DHCP_MSG_ACK || resp_msg_type == DHCP_MSG_OFFER) {
        struct in_addr addr = {.s_addr = assigned_ip};
        dpinfo("Assigned IP: %s", inet_ntoa(addr));
    } else if (resp_msg_type == DHCP_MSG_NAK) {
        dpinfo("Sending NAK");
    }
}

/****************************************************************************
 * UDP Receive Callback
 ****************************************************************************/
static void dhcp_recv_cb(uv_udp_t *handle, ssize_t nread,
                        const uv_buf_t *buf,
                        const struct sockaddr *addr,
                        unsigned flags)
{
    (void)handle;
    (void)flags;

    if (nread < 0) {
        dperr("recv error: %s", uv_strerror(nread));
        return;
    }

    if (nread == 0 || addr == NULL) {
        return;
    }

    uint8_t resp_buf[DHCP_MAX_LEN];
    ssize_t resp_len = 0;
    
    dhcp_process_request((const uint8_t *)buf->base, nread,
                         resp_buf, &resp_len, (struct sockaddr_in *)addr);

    if (resp_len > 0) {
        uv_udp_send_t send_req;
        uv_buf_t resp_buf_vec = uv_buf_init((char *)resp_buf, resp_len);
        
        int ret = uv_udp_send(&send_req, handle, &resp_buf_vec, 1,
                              addr, NULL);
        if (ret < 0) {
            dperr("send error: %s", uv_strerror(ret));
        }
    }
}

/****************************************************************************
 * Alloc callback for UDP
 ****************************************************************************/
static void dhcp_alloc_cb(uv_handle_t *handle, size_t suggested_size,
                          uv_buf_t *buf)
{
    (void)handle;
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

/****************************************************************************
 * Close callback
 ****************************************************************************/
static void dhcp_close_cb(uv_handle_t *handle)
{
    (void)handle;
    dpinfo("DHCP server closed");
}

/* Global functions ---------------------------------------------------------*/
/****************************************************************************
 * Start DHCP Server
 ****************************************************************************/
int dhcp_server_start(uv_loop_t *loop, const char *interface,
                     in_addr_t server_ip, in_addr_t start_ip,
                     in_addr_t end_ip, in_addr_t netmask)
{
    (void)interface;

    if (g_dhcp_ctx.running) {
        dperr("DHCP server already running");
        return -EBUSY;
    }

    memset(&g_dhcp_ctx, 0, sizeof(g_dhcp_ctx));
    g_dhcp_ctx.server_ip = server_ip;
    g_dhcp_ctx.start_ip = start_ip;
    g_dhcp_ctx.end_ip = end_ip;
    g_dhcp_ctx.netmask = netmask;
    g_dhcp_ctx.router_ip = server_ip;
    g_dhcp_ctx.lease_time = DHCP_DEFAULT_LEASE;
    g_dhcp_ctx.running = true;

    int ret = uv_udp_init(loop, &g_dhcp_ctx.udp_handle);
    if (ret < 0) {
        dperr("uv_udp_init failed: %s", uv_strerror(ret));
        return ret;
    }

    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;
    bind_addr.sin_port = htons(DHCP_SERVER_PORT);

    ret = uv_udp_bind(&g_dhcp_ctx.udp_handle,
                      (const struct sockaddr *)&bind_addr, 0);
    if (ret < 0) {
        dperr("uv_udp_bind failed: %s", uv_strerror(ret));
        return ret;
    }

    ret = uv_udp_recv_start(&g_dhcp_ctx.udp_handle, dhcp_alloc_cb, dhcp_recv_cb);
    if (ret < 0) {
        dperr("uv_udp_recv_start failed: %s", uv_strerror(ret));
        return ret;
    }

    struct in_addr addr = {.s_addr = server_ip};
    struct in_addr start = {.s_addr = start_ip};
    struct in_addr end = {.s_addr = end_ip};
    dpinfo("DHCP server started, server=%s, pool=%s-%s",
              inet_ntoa(addr), inet_ntoa(start), inet_ntoa(end));

    return 0;
}

/****************************************************************************
 * Stop DHCP Server
 ****************************************************************************/
void dhcp_server_stop(void)
{
    if (!g_dhcp_ctx.running) {
        return;
    }

    g_dhcp_ctx.running = false;

    uv_close((uv_handle_t *)&g_dhcp_ctx.udp_handle, dhcp_close_cb);

    dpinfo("DHCP server stopping");
}

/****************************************************************************
 * Get server status
 ****************************************************************************/
bool dhcp_server_is_running(void)
{
    return g_dhcp_ctx.running;
}
