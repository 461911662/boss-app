/**
 * @file allocator.h
 * @brief 移植谷歌开源软件
 * @attention 请遵循开源软件协议
 */

#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include <stddef.h>
#include <stdlib.h>

/****************************************************************************
 * DEFINES
 ****************************************************************************/
#if defined HEAP_MEMORY_DEBUG
void osi_mem_dbg_init(void);
void osi_mem_dbg_record(void *p, int size, const char *func, int line);
void osi_mem_dbg_clean(void *p, const char *func, int line);
void osi_mem_dbg_show(void);
uint32_t osi_mem_dbg_get_max_size(void);
uint32_t osi_mem_dbg_get_current_size(void);
void osi_men_dbg_set_section_start(uint8_t index);
void osi_men_dbg_set_section_end(uint8_t index);
uint32_t osi_mem_dbg_get_max_size_section(uint8_t index);

#define osi_malloc(size)                                \
({                                                      \
    void *p;                                            \
    p = malloc((size));                                 \
    osi_mem_dbg_record(p, size, __func__, __LINE__);    \
    (void *)p;                                          \
})

#define osi_calloc(size)                                \
({                                                      \
    void *p;                                            \
    p = calloc(1, (size));                              \
    osi_mem_dbg_record(p, size, __func__, __LINE__);    \
    (void *)p;                                          \
})

#define osi_free(ptr)                                   \
do {                                                    \
    void *tmp_point = (void *)(ptr);                    \
    osi_mem_dbg_clean(tmp_point, __func__, __LINE__);   \
    free(tmp_point);                                    \
} while (0)

#else
#define osi_malloc(size)                  malloc((size))
#define osi_calloc(size)                  calloc(1, (size))
#define osi_free(p)                       free((p))
#endif /* HEAP_MEMORY_DEBUG */

#define FREE_AND_RESET(a)   \
do {                        \
    if (a) {                \
        osi_free(a);        \
        a = NULL;           \
    }                       \
}while (0)


/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/
char *osi_strdup(const char *str);
void *osi_malloc_func(size_t size);
void *osi_calloc_func(size_t size);
void osi_free_func(void *ptr);

#endif /* _ALLOCATOR_H_ */
