/**
 * @file include/watchui/log.h
 * @brief 日志接口
*/

#ifndef _WATCHUI_DEBUG_LOG_H_
#define _WATCHUI_DEBUG_LOG_H_

#ifndef __ASSEMBLY__

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/* Global enums ------------------------------------------------------------- */
/* Global structs ----------------------------------------------------------- */
/* Global typedefs ---------------------------------------------------------- */
/* Global variables --------------------------------------------------------- */
/* Global variable prototypes ----------------------------------------------- */
/* Global function prototypes ----------------------------------------------- */

/* Includes ----------------------------------------------------------------- */

#include <syslog.h>

/* Global defines ----------------------------------------------------------- */

#define	LOG_ERR		3	/* error conditions, refs syslog level */
#define	LOG_WARNING	4	/* warning conditions, refs syslog level */
#define	LOG_INFO	6	/* informational, refs syslog level */
#define	LOG_DEBUG	7	/* debug-level messages, refs syslog level */

#ifdef CONFIG_WATCHUI_DEBUG_LOG

#ifdef CONFIG_CPP_HAVE_VARARGS
#  define apperr(format, ...) \
    do { syslog(LOG_ERR, format, ##__VA_ARGS__); } while (0)
#else
#  define apperr           (void)
#endif

#ifdef CONFIG_CPP_HAVE_VARARGS
#  define appwarn(format, ...) \
    do { syslog(LOG_WARNING, format, ##__VA_ARGS__); } while (0)
#else
#  define appwarn          (void)
#endif

#ifdef CONFIG_CPP_HAVE_VARARGS
#  define appinfo(format, ...) \
    do { syslog(LOG_INFO, format, ##__VA_ARGS__); } while (0)
#else
#  define appinfo          (void)
#endif

#ifdef CONFIG_CPP_HAVE_VARARGS
#  define appdbg(format, ...) \
    do { syslog(LOG_DEBUG, format, ##__VA_ARGS__); } while (0)
#else
#  define appdbg           (void)
#endif

#else

#  define apperr           (void)
#  define appwarn          (void)
#  define appinfo          (void)
#  define appdbg           (void)

#endif //!CONFIG_WATCHUI_DEBUG_LOG

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* _WATCHUI_DEBUG_LOG_H_ */