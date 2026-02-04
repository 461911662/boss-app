/**
 * @file watch-ui/entry/watchui_c2cxx.h
 * @brief c++ language interface to c
*/

#ifndef _WATCHUI_C2CXX_H_
#define _WATCHUI_C2CXX_H_

#ifndef __ASSEMBLY__

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/* Global defines ------------------------------------------------------------ */
/* Global enums -------------------------------------------------------------- */
/* Global structs ------------------------------------------------------------ */
/* Global typedefs ----------------------------------------------------------- */
/* Global variables ---------------------------------------------------------- */
/* Global variable prototypes ----------------------------------------------- */

/* Global function prototypes ----------------------------------------------- */

/* Includes ------------------------------------------------------------------ */

#include <nuttx/config.h>
#include <nuttx/compiler.h>

/*
 * @brief: watchui_main 应用入口函数
 * @param: argc 参数数量
 * @param: argv 参数数组
 * @return: 0
*/
EXTERN int watchui_main(int argc, char *argv[]);

/**
 * @brief watchui 主线程异步触发
 * @return 无
*/
EXTERN void watchui_async_send();

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* _WATCHUI_C2CXX_H_ */
