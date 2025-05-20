/**
 * @file hash_functions.h
 * @brief 移植components\bt\common\osi\include\hash_functions.h
 * @attention 可以自由学习
 */


#ifndef _HASH_FUNCTIONS_H_
#define _HASH_FUNCTIONS_H_

/****************************************************************************
 * INCLUDES
 ****************************************************************************/

#include "osi/hash_map.h"

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/

typedef unsigned char hash_key_t[4];

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/

hash_index_t hash_function_naive(const void *key);

hash_index_t hash_function_integer(const void *key);

// Hashes a pointer based only on its address value
hash_index_t hash_function_pointer(const void *key);

hash_index_t hash_function_string(const void *key);

void hash_function_blob(const unsigned char *s, unsigned int len, hash_key_t h);

#endif /* _HASH_FUNCTIONS_H_ */
