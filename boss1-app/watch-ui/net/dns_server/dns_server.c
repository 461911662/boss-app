/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/dns_server/dns_server.c
 *
 * Captive Portal DNS Server for NuttX
 * Simplified version - only handles A record queries, all domains point to portal IP
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

#include "dns_server.h"
#include "watchui/log.h"

/* Private defines ----------------------------------------------------------*/

/* Check configuration.  This is not all of the configuration settings that
 * are required -- only the more obvious.
 */

#ifndef CONFIG_LIBUV
#    error "You must select CONFIG_LIBUV in your configuration file"
#endif

/* DNS Protocol Constants */
#define DNS_PORT        53
#define DNS_MAX_LEN     512
#define DNS_QR_RESPONSE (1 << 15)   /* Response flag */
#define DNS_OPCODE_MASK 0x7800
#define DNS_TYPE_A      1
#define DNS_TYPE_AAAA   28
#define DNS_CLASS_IN    1
#define DNS_TTL         300

/* Logging macros */
#define dnsinfo(fmt, ...)    appinfo("[DNS] " fmt, ##__VA_ARGS__)
#define dnsdbg(fmt, ...)     appdbg("[DNS] " fmt, ##__VA_ARGS__)
#define dnswarn(fmt, ...)    appwarn("[DNS] " fmt, ##__VA_ARGS__)
#define dnserr(fmt, ...)     apperr("[DNS] " fmt, ##__VA_ARGS__)

/* Private typedefs ----------------------------------------------------------*/

/* DNS Header Structure */
typedef struct __attribute__((packed)) {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;   /* Question count */
    uint16_t ancount;   /* Answer count */
    uint16_t nscount;   /* Authority count */
    uint16_t arcount;   /* Additional count */
} dns_header_t;

/* DNS Question Structure */
typedef struct __attribute__((packed)) {
    uint16_t type;
    uint16_t class;
} dns_question_t;

/* DNS Answer Structure (A Record) */
typedef struct __attribute__((packed)) {
    uint16_t ptr;       /* Pointer to question name */
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t len;
    uint32_t ip;        /* IPv4 address */
} dns_answer_t;

/* DNS Server Context */
typedef struct {
    int sock;                       /* UDP socket */
    in_addr_t portal_ip;            /* Portal IP (AP IP) */
    bool running;                   /* Server running flag */
    uv_poll_t poll_handle;          /* libuv poll handle */
    uint8_t *req_buf;               /* DNS request buffer */
    uint8_t *resp_buf;              /* DNS response buffer */
    uint32_t buf_size;              /* Request/response buffer size */
} dns_server_ctx_t;


/* Private variables --------------------------------------------------------*/

static dns_server_ctx_t g_dns_ctx = {
    .sock = -1,
    .running = false
};

/* Private functions --------------------------------------------------------*/

/**
 * @details Extract domain name from DNS packet
 * @param[in] buf: DNS packet buffer
 * @param[in] buf_len: DNS packet length
 * @param[in] offset: Offset to start parsing
 * @param[out] name_out: Output buffer for domain name
 * @param[in] name_max: Maximum length of output buffer
 * @return Length of domain name or -1 on error
*/
static int dns_parse_name(const uint8_t *buf, int buf_len, int offset, 
                          char *name_out, int name_max)
{
    int name_len = 0;
    int pos = offset;
    int jumped = 0;
    int jump_count = 0;
    const int MAX_JUMPS = 10;
    
    name_out[0] = '\0';
    
    while (pos < buf_len && jump_count < MAX_JUMPS) {
        uint8_t len = buf[pos];
        
        /* Check for compression pointer (0xC0) */
        if ((len & 0xC0) == 0xC0) {
            if (pos + 1 >= buf_len) return -1;
            int new_offset = ((len & 0x3F) << 8) | buf[pos + 1];
            if (!jumped) jumped = pos + 2;
            pos = new_offset;
            jump_count++;
            continue;
        }
        
        /* End of name */
        if (len == 0) {
            if (!jumped) jumped = pos + 1;
            break;
        }
        
        pos++;
        
        /* Check bounds */
        if (pos + len > buf_len || name_len + len + 1 > name_max) {
            return -1;
        }
        
        /* Copy label */
        if (name_len > 0) {
            name_out[name_len++] = '.';
        }
        memcpy(name_out + name_len, buf + pos, len);
        name_len += len;
        name_out[name_len] = '\0';
        
        pos += len;
    }
    
    return jumped > 0 ? jumped : pos + 1;
}


/**
 * @details Process DNS request and generate response
 * @param[in] req: DNS request buffer
 * @param[in] req_len: DNS request length
 * @param[out] resp: DNS response buffer
 * @param[in] resp_max: Maximum length of response buffer
 * @return 0 if no response, -1 if error, >0 if response length
 */
static int dns_process_request(const uint8_t *req, int req_len, 
                               uint8_t *resp, int resp_max)
{
    if (req_len < (int)sizeof(dns_header_t)) {
        dnserr("Request too short: %d bytes", req_len);
        return -1;
    }
    
    if (req_len > resp_max) {
        dnserr("Request too large: %d bytes", req_len);
        return -1;
    }
    
    const dns_header_t *req_hdr = (const dns_header_t *)req;
    uint16_t flags = ntohs(req_hdr->flags);
    uint16_t qdcount = ntohs(req_hdr->qdcount);
    
    /* Only handle standard queries */
    if ((flags & DNS_OPCODE_MASK) != 0) {
        dnsinfo("Non-standard query, opcode: 0x%04X", flags & DNS_OPCODE_MASK);
        return 0;
    }
    
    /* Copy request to response buffer */
    memcpy(resp, req, req_len);
    dns_header_t *resp_hdr = (dns_header_t *)resp;
    
    /* Set response flags */
    resp_hdr->flags = htons(flags | DNS_QR_RESPONSE);
    resp_hdr->ancount = 0;  /* Will be updated */
    resp_hdr->nscount = 0;
    resp_hdr->arcount = 0;
    
    int resp_len = req_len;
    int answer_count = 0;
    
    /* Parse each question */
    int qoffset = sizeof(dns_header_t);
    for (int i = 0; i < qdcount && i < 1; i++) {  /* Only answer first question for simplicity */
        char name[256];
        int name_end = dns_parse_name(req, req_len, qoffset, name, sizeof(name));
        
        if (name_end < 0) {
            dnserr("Failed to parse question name");
            return -1;
        }
        
        if (name_end + (int)sizeof(dns_question_t) > req_len) {
            dnserr("Truncated question");
            return -1;
        }
        
        const dns_question_t *q = (const dns_question_t *)(req + name_end);
        uint16_t qtype = ntohs(q->type);
        uint16_t qclass = ntohs(q->class);
        
        dnsinfo("Query: %s (type=%s, class=%d)", 
                 name, 
                 qtype == DNS_TYPE_A ? "A" : (qtype == DNS_TYPE_AAAA ? "AAAA" : "OTHER"),
                 qclass);
        
        /* Only answer A records (IPv4) */
        if (qtype == DNS_TYPE_A && qclass == DNS_CLASS_IN) {
            /* Check if we have room for answer */
            if (resp_len + (int)sizeof(dns_answer_t) > resp_max) {
                dnserr("Response buffer full");
                break;
            }
            
            dns_answer_t *ans = (dns_answer_t *)(resp + resp_len);
            
            ans->ptr = htons(0xC000 | qoffset);  /* Pointer to question name */
            ans->type = htons(DNS_TYPE_A);
            ans->class = htons(DNS_CLASS_IN);
            ans->ttl = htonl(DNS_TTL);
            ans->len = htons(4);
            ans->ip = g_dns_ctx.portal_ip;
            
            resp_len += sizeof(dns_answer_t);
            answer_count++;
            
            struct in_addr addr = {.s_addr = g_dns_ctx.portal_ip};
            dnsinfo("Reply: %s -> %s", name, inet_ntoa(addr));
        }
        
        qoffset = name_end + sizeof(dns_question_t);
    }
    
    resp_hdr->ancount = htons(answer_count);
    return resp_len;
}


/**
 * @details Close the socket when handle is closed
 * @param[in] handle: libuv handle
 * @return void
*/
static void dns_close_cb(uv_handle_t *handle)
{
    (void)handle;
    if (g_dns_ctx.sock >= 0) {
        close(g_dns_ctx.sock);
        g_dns_ctx.sock = -1;
    }
    dnsinfo("DNS server stopped");
}


/**
 * @details libuv Poll Callback
 * @param[in] handle: libuv handle
 * @param[in] status: status code
 * @param[in] events: event flags
 * @return void
*/
static void dns_poll_cb(uv_poll_t *handle, int status, int events)
{
    if (status < 0) {
        dnserr("Poll error: %s", uv_strerror(status));
        return;
    }
    
    if (!(events & UV_READABLE)) {
        return;
    }

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    /* Receive request */
    ssize_t req_len = recvfrom(g_dns_ctx.sock, g_dns_ctx.req_buf, g_dns_ctx.buf_size, 0,
                               (struct sockaddr *)&client_addr, &addr_len);
    
    if (req_len < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            dnserr("recvfrom failed: %d", errno);
        }
        return;
    }
    
    /* Process request */
    int resp_len = dns_process_request(g_dns_ctx.req_buf, (int)req_len,
                                        g_dns_ctx.resp_buf, g_dns_ctx.buf_size);
    
    if (resp_len > 0) {
        /* Send response */
        ssize_t sent = sendto(g_dns_ctx.sock, g_dns_ctx.resp_buf, resp_len, 0,
                              (struct sockaddr *)&client_addr, addr_len);
        
        if (sent < 0) {
            dnserr("sendto failed: %d", errno);
        }
    }
}

/* Global functions -------------------------------------------------------*/

/**
 * @details Start DNS server
 * @param[in] loop: libuv loop
 * @param[in] portal_ip: IP address of the portal
 * @return 0 on success, negative error code on failure
*/
int dns_server_start(uv_loop_t *loop, in_addr_t portal_ip)
{
    if (g_dns_ctx.running) {
        dnserr("DNS server already running");
        return -EBUSY;
    }
    
    memset(&g_dns_ctx, 0, sizeof(g_dns_ctx));
    g_dns_ctx.portal_ip = portal_ip;
    
    /* Create UDP socket */
    g_dns_ctx.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (g_dns_ctx.sock < 0) {
        dnserr("socket failed: %d", errno);
        return -errno;
    }
    
    /* Set non-blocking */
    int flags = fcntl(g_dns_ctx.sock, F_GETFL, 0);
    fcntl(g_dns_ctx.sock, F_SETFL, flags | O_NONBLOCK);
    
    /* Allow address reuse */
    int reuse = 1;
    setsockopt(g_dns_ctx.sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    /* Bind to port 53 */
    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;
    bind_addr.sin_port = htons(DNS_PORT);
    
    if (bind(g_dns_ctx.sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
        dnserr("bind failed: %d", errno);
        close(g_dns_ctx.sock);
        return -errno;
    }

    /* Malloc req/resp buf */
    g_dns_ctx.buf_size = DNS_MAX_LEN;
    g_dns_ctx.req_buf = malloc(DNS_MAX_LEN);
    if (g_dns_ctx.req_buf == NULL) {
        dnserr("req_buf malloc failed");
        close(g_dns_ctx.sock);
        return -ENOMEM;
    }

    g_dns_ctx.resp_buf = malloc(DNS_MAX_LEN);
    if (g_dns_ctx.resp_buf == NULL) {
        dnserr("resp_buf malloc failed");
        free(g_dns_ctx.req_buf);
        close(g_dns_ctx.sock);
        return -ENOMEM;
    }
    
    /* Setup libuv poll */
    int ret = uv_poll_init(loop, &g_dns_ctx.poll_handle, g_dns_ctx.sock);
    if (ret < 0) {
        dnserr("uv_poll_init failed: %s", uv_strerror(ret));
        close(g_dns_ctx.sock);
        return ret;
    }
    
    g_dns_ctx.poll_handle.data = &g_dns_ctx;
    
    ret = uv_poll_start(&g_dns_ctx.poll_handle, UV_READABLE, dns_poll_cb);
    if (ret < 0) {
        dnserr("uv_poll_start failed: %s", uv_strerror(ret));
        uv_close((uv_handle_t *)&g_dns_ctx.poll_handle, NULL);
        close(g_dns_ctx.sock);
        return ret;
    }
    
    g_dns_ctx.running = true;
    
    struct in_addr addr = {.s_addr = portal_ip};
    dnsinfo("Server started on port %d, portal IP: %s", DNS_PORT, inet_ntoa(addr));
    
    return 0;
}


/**
 * @details Stop DNS server
 * @return void
*/
void dns_server_stop(void)
{
    if (!g_dns_ctx.running) {
        return;
    }
    
    g_dns_ctx.running = false;
    
    /* Stop poll first */
    uv_poll_stop(&g_dns_ctx.poll_handle);
    
    /* Close the handle with callback to clean up socket */
    uv_close((uv_handle_t *)&g_dns_ctx.poll_handle, dns_close_cb);

    /* Free req/resp buf */
    free(g_dns_ctx.req_buf);
    free(g_dns_ctx.resp_buf);
    memset(&g_dns_ctx, 0, sizeof(g_dns_ctx));
    dnsinfo("Server resources cleaned up");
}


/**
 * @details Check if DNS server is running
 * @return true if running, false otherwise
*/
bool dns_server_is_running(void)
{
    return g_dns_ctx.running;
}
