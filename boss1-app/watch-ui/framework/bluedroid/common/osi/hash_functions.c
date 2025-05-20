/**
 * @file hash_functions.c
 * @brief 移植components\bt\common\osi\hash_functions.c
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/

#include <string.h>

#include "osi/hash_functions.h"

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/

hash_index_t hash_function_naive(const void *key)
{
    return (hash_index_t)key;
}

hash_index_t hash_function_integer(const void *key)
{
    return ((hash_index_t)key) * 2654435761;
}

hash_index_t hash_function_pointer(const void *key)
{
    return ((hash_index_t)key) * 2654435761;
}

hash_index_t hash_function_string(const void *key)
{
    hash_index_t hash = 5381;
    const char *name = (const char *)key;
    size_t string_len = strlen(name);
    for (size_t i = 0; i < string_len; ++i) {
        hash = ((hash << 5) + hash ) + name[i];
    }
    return hash;
}

void hash_function_blob(const unsigned char *s, unsigned int len, hash_key_t h)
{
    size_t j;

    while (len--) {
        j = sizeof(hash_key_t)-1;

        while (j) {
            h[j] = ((h[j] << 7) | (h[j-1] >> 1)) + h[j];
            --j;
        }

        h[0] = (h[0] << 7) + h[0] + *s++;
    }
}
