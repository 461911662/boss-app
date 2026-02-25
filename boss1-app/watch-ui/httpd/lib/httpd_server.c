/****************************************************************************
 * vendor/boss/app/boss1-app/watch-ui/net/httpd/httpd_server.c
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <netinet/in.h>
#include <nuttx/sched.h>

#include <time.h>

#include "netutils/thttpd.h"
#include "httpd_server.h"
#include "libhttpd.h"

#include <nuttx/drivers/ramdisk.h>
#  include <sys/boardctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Check configuration.  This is not all of the configuration settings that
 * are required -- only the more obvious.
 */

#ifdef CONFIG_BINFMT_DISABLE
#  error "You must not disable loadable modules via CONFIG_BINFMT_DISABLE in your configuration file"
#endif

#ifdef CONFIG_MY_HTTPD_BINFS
#  ifndef CONFIG_BUILTIN
#    error "You must select CONFIG_BUILTIN=y in your configuration file"
#  endif

#  ifndef CONFIG_FS_BINFS
#    error "You must select CONFIG_FS_BINFS=y in your configuration file"
#  endif

#  ifndef CONFIG_FS_UNIONFS
#    error "CONFIG_FS_UNIONFS=y is required in this configuration"
#  endif
#endif

/* Describe the ROMFS file system */

#define SECTORSIZE   64
#define NSECTORS(b)  (((b)+SECTORSIZE-1)/SECTORSIZE)
#define ROMFSDEV     "/dev/ram0"

#ifdef CONFIG_MY_HTTPD_BINFS
#  define ROMFS_MOUNTPT      "/mnt/tmp1"
#  define ROMFS_PREFIX       ""
#  define BINFS_MOUNTPT      "/mnt/tmp2"
#  define BINFS_PREFIX       "cgi-bin"
#  define UNIONFS_MOUNTPT    CONFIG_MY_HTTPD_PATH
#endif

/* HTTPD Server Context */
typedef struct {
    in_addr_t server_ip;
    bool running;
    pid_t pid;
} httpd_server_ctx_t;

static httpd_server_ctx_t g_httpd_ctx = {
    .running = false,
    .pid = -1,
};


/****************************************************************************
 * Symbols from Auto-Generated Code
 ****************************************************************************/

extern const unsigned char romfs_img[];
extern const unsigned int romfs_img_len;

int httpd_server_start(void)
{
  char *thttpd_argv[] = { "thttpd", NULL };
  int ret;

#ifdef CONFIG_MY_HTTPD_BINFS
  struct boardioc_romdisk_s desc;
  printf("Registering romdisk\n");

  desc.minor    = 0;
  desc.nsectors = NSECTORS(romfs_img_len);
  desc.sectsize = SECTORSIZE;
  desc.image    = (FAR uint8_t *)romfs_img;

  ret = boardctl(BOARDIOC_ROMDISK, (uintptr_t)&desc);

  if (ret < 0)
    {
      printf("ERROR: romdisk_register failed: %d\n", ret);
      exit(1);
    }

  printf("Mounting ROMFS filesystem at target=%s with source=%s\n",
         ROMFS_MOUNTPT, ROMFSDEV);

  ret = mount(ROMFSDEV, ROMFS_MOUNTPT, "romfs", MS_RDONLY, NULL);
  if (ret < 0)
    {
      printf("ERROR: mount(%s,%s,romfs) failed: %d\n",
             ROMFSDEV, ROMFS_MOUNTPT, errno);
    }

  printf("Mounting BINFS filesystem at %s\n", BINFS_MOUNTPT);

  ret = mount(NULL, BINFS_MOUNTPT, "binfs", MS_RDONLY, NULL);
  if (ret < 0)
    {
      printf("ERROR: mount(NULL,%s,binfs) failed: %d\n",
             BINFS_MOUNTPT, errno);
    }

  printf("Creating UNIONFS filesystem at %s\n", UNIONFS_MOUNTPT);

  ret = mount(NULL, UNIONFS_MOUNTPT, "unionfs", 0,
              "fspath1=" ROMFS_MOUNTPT ",prefix1=" ROMFS_PREFIX
              ",fspath2=" BINFS_MOUNTPT ",prefix2=" BINFS_PREFIX);
  if (ret < 0)
    {
      printf("ERROR: Failed to create the union file system at %s: %d\n",
             UNIONFS_MOUNTPT, ret);
    }
#endif

#ifdef CONFIG_MY_HTTPD_CAPTIVE_PORTAL
  captive_portal_enable(true);
#endif

  printf("Starting THTTPD\n");

  g_httpd_ctx.pid = task_create("mythttpd",
                                CONFIG_MY_HTTPD_PRIORITY,
                                CONFIG_MY_HTTPD_STACKSIZE,
                                (main_t)thttpd_main,
                                (char *const*)thttpd_argv);

  if (g_httpd_ctx.pid < 0)
    {
      printf("ERROR: Failed to start thttpd: %d\n", errno);
      return g_httpd_ctx.pid;
    }

  g_httpd_ctx.running = true;
  printf("THTTPD started with PID: %d\n", g_httpd_ctx.pid);

  return 0;
}

void httpd_server_stop(void)
{
    if (g_httpd_ctx.running && g_httpd_ctx.pid > 0)
    {
        printf("Stopping THTTPD (PID: %d)\n", g_httpd_ctx.pid);
        task_delete(g_httpd_ctx.pid);
        g_httpd_ctx.pid = -1;
        g_httpd_ctx.running = false;
    }
}

bool httpd_server_is_running(void)
{
  return g_httpd_ctx.running;
}
