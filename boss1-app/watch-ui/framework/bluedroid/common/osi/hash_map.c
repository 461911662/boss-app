/**
 * @file hash_map.c
 * @brief 移植谷歌开源软件
 * @attention 请遵循开源软件协议
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/

#include "bt_common.h"
#include "osi/list.h"
#include "osi/hash_map.h"
#include "osi/allocator.h"

/****************************************************************************
 * STRUCTS
 ****************************************************************************/

struct hash_map_t;


/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/

typedef struct hash_map_bucket_t {
    list_t *list;
} hash_map_bucket_t;

typedef struct hash_map_t {
    hash_map_bucket_t *bucket;
    size_t num_bucket;
    size_t hash_size;
    hash_index_fn hash_fn;
    key_free_fn key_fn;
    data_free_fn data_fn;
    key_equality_fn keys_are_equal;
} hash_map_t;

/****************************************************************************
 * STATIC PROTOTYPES
 ****************************************************************************/

static void bucket_free_(void *data);
static bool default_key_equality(const void *x, const void *y);
static hash_map_entry_t *find_bucket_entry_(list_t *hash_bucket_list,
        const void *key);

/****************************************************************************
 * GLOBAL FUNCTIONS
 ****************************************************************************/

// Hidden constructor for list, only to be used by us.
list_t *list_new_internal(list_free_cb callback);

// Hidden constructor, only to be used by the allocation tracker. Behaves the same as
// |hash_map_new|, except you get to specify the allocator.
hash_map_t *hash_map_new_internal(
    size_t num_bucket,
    hash_index_fn hash_fn,
    key_free_fn key_fn,
    data_free_fn data_fn,
    key_equality_fn equality_fn)
{
    assert(hash_fn != NULL);
    assert(num_bucket > 0);
    hash_map_t *hash_map = osi_calloc(sizeof(hash_map_t));
    if (hash_map == NULL) {
        return NULL;
    }

    hash_map->hash_fn = hash_fn;
    hash_map->key_fn = key_fn;
    hash_map->data_fn = data_fn;
    hash_map->keys_are_equal = equality_fn ? equality_fn : default_key_equality;

    hash_map->num_bucket = num_bucket;
    hash_map->bucket = osi_calloc(sizeof(hash_map_bucket_t) * num_bucket);
    if (hash_map->bucket == NULL) {
        osi_free(hash_map);
        return NULL;
    }
    return hash_map;
}

/**
 * 创建一个hash map对象
 * @param num_bucket 表示桶的大小，内存块大小
 * @param hash_fn 用于计算键的哈希值并映射到桶索引
 * @param key_fn 键释放函数，用于在删除键时清理内存
 * @param data_fn 数据释放函数，用于在删除数据时清理内存
 * @param equality_fn 键比较函数，用于判断两个键是否相等(如 strcmp)
 * @return 成功时返回hash map对象，失败时返回NULL
 */
hash_map_t *hash_map_new(
    size_t num_bucket,
    hash_index_fn hash_fn,
    key_free_fn key_fn,
    data_free_fn data_fn,
    key_equality_fn equality_fn)
{
    return hash_map_new_internal(num_bucket, hash_fn, key_fn, data_fn, equality_fn);
}

/**
 * 释放一个hash map对象
 * @param hash_map表示需要释放的hash map对象
 * @return 无
 */
void hash_map_free(hash_map_t *hash_map)
{
    if (hash_map == NULL) {
        return;
    }
    hash_map_clear(hash_map);
    osi_free(hash_map->bucket);
    osi_free(hash_map);
}

/*
bool hash_map_is_empty(const hash_map_t *hash_map) {
  assert(hash_map != NULL);
  return (hash_map->hash_size == 0);
}

size_t hash_map_size(const hash_map_t *hash_map) {
  assert(hash_map != NULL);
  return hash_map->hash_size;
}

size_t hash_map_num_buckets(const hash_map_t *hash_map) {
  assert(hash_map != NULL);
  return hash_map->num_bucket;
}
*/

/**
 * 判断hash map的桶中是否有指定的key
 * @param hash_map表示需要查找的hash map对象
 * @param key表示传入的hash key参数
 * @return 成功返回true, 失败返回false
 */
bool hash_map_has_key(const hash_map_t *hash_map, const void *key)
{
    assert(hash_map != NULL);

    hash_index_t hash_key = hash_map->hash_fn(key) % hash_map->num_bucket;
    list_t *hash_bucket_list = hash_map->bucket[hash_key].list;

    hash_map_entry_t *hash_map_entry = find_bucket_entry_(hash_bucket_list, key);
    return (hash_map_entry != NULL);
}

/**
 * 在hash map中添加指定的数据
 * @param hash_map表示指定的hash map对象
 * @param key表示指定的hash key参数
 * @param data表示需要加入的数据
 * @return 成功返回true, 失败返回false
 */
bool hash_map_set(hash_map_t *hash_map, const void *key, void *data)
{
    assert(hash_map != NULL);
    assert(data != NULL);

    hash_index_t hash_key = hash_map->hash_fn(key) % hash_map->num_bucket;

    if (hash_map->bucket[hash_key].list == NULL) {
        hash_map->bucket[hash_key].list = list_new_internal(bucket_free_);
        if (hash_map->bucket[hash_key].list == NULL) {
            return false;
        }
    }
    list_t *hash_bucket_list = hash_map->bucket[hash_key].list;

    hash_map_entry_t *hash_map_entry = find_bucket_entry_(hash_bucket_list, key);

    if (hash_map_entry) {
        // Calls hash_map callback to delete the hash_map_entry.
        bool rc = osi_list_remove(hash_bucket_list, hash_map_entry);
        assert(rc == true);
        (void)rc;
    } else {
        hash_map->hash_size++;
    }
    hash_map_entry = osi_calloc(sizeof(hash_map_entry_t));
    if (hash_map_entry == NULL) {
        return false;
    }

    hash_map_entry->key = key;
    hash_map_entry->data = data;
    hash_map_entry->hash_map = hash_map;

    return osi_list_append(hash_bucket_list, hash_map_entry);
}

/**
 * 在hash map中擦除指定的数据
 * @param hash_map 表示指定的hash map对象
 * @param key 表示指定的hash key参数
 * @return 成功返回true, 失败返回false
 */
bool hash_map_erase(hash_map_t *hash_map, const void *key)
{
    assert(hash_map != NULL);

    hash_index_t hash_key = hash_map->hash_fn(key) % hash_map->num_bucket;
    list_t *hash_bucket_list = hash_map->bucket[hash_key].list;

    hash_map_entry_t *hash_map_entry = find_bucket_entry_(hash_bucket_list, key);
    if (hash_map_entry == NULL) {
        return false;
    }

    hash_map->hash_size--;
    bool remove = osi_list_remove(hash_bucket_list, hash_map_entry);
    if(osi_list_is_empty(hash_map->bucket[hash_key].list)) {
        osi_list_free(hash_map->bucket[hash_key].list);
        hash_map->bucket[hash_key].list = NULL;
    }

    return remove;
}

/**
 * 在hash map中获取指定的数据
 * @param hash_map表示指定的hash map对象
 * @param key表示指定的hash key参数
 * @return 成功返回指定数据, 失败返回NULL
 */
void *hash_map_get(const hash_map_t *hash_map, const void *key)
{
    assert(hash_map != NULL);

    hash_index_t hash_key = hash_map->hash_fn(key) % hash_map->num_bucket;
    list_t *hash_bucket_list = hash_map->bucket[hash_key].list;

    hash_map_entry_t *hash_map_entry = find_bucket_entry_(hash_bucket_list, key);
    if (hash_map_entry != NULL) {
        return hash_map_entry->data;
    }

    return NULL;
}

/**
 * 释放hash map的桶内存中的数据
 * @param hash_map 表示需要释放的hash map对象
 * @return 无
 */
void hash_map_clear(hash_map_t *hash_map)
{
    assert(hash_map != NULL);

    for (hash_index_t i = 0; i < hash_map->num_bucket; i++) {
        if (hash_map->bucket[i].list == NULL) {
            continue;
        }
        osi_list_free(hash_map->bucket[i].list);
        hash_map->bucket[i].list = NULL;
    }
}

/**
 * hash map的遍历
 * @param hash_map 表示需要遍历的hash map对象
 * @param callback 表示遍历时调用的回调函数
 * @param context 表示遍历时回调函数传入的上下文
 * @return 无
 */
void hash_map_foreach(hash_map_t *hash_map, hash_map_iter_cb callback, void *context)
{
    assert(hash_map != NULL);
    assert(callback != NULL);

    for (hash_index_t i = 0; i < hash_map->num_bucket; ++i) {
        if (hash_map->bucket[i].list == NULL) {
            continue;
        }
        for (const list_node_t *iter = osi_list_begin(hash_map->bucket[i].list);
                iter != osi_list_end(hash_map->bucket[i].list);
                iter = osi_list_next(iter)) {
            hash_map_entry_t *hash_map_entry = (hash_map_entry_t *)osi_list_node(iter);
            if (!callback(hash_map_entry, context)) {
                return;
            }
        }
    }
}

/****************************************************************************
 * STATIC FUNCTIONS
 ****************************************************************************/

static void bucket_free_(void *data)
{
    assert(data != NULL);
    hash_map_entry_t *hash_map_entry = (hash_map_entry_t *)data;
    const hash_map_t *hash_map = hash_map_entry->hash_map;

    if (hash_map->key_fn) {
        hash_map->key_fn((void *)hash_map_entry->key);
    }
    if (hash_map->data_fn) {
        hash_map->data_fn(hash_map_entry->data);
    }
    osi_free(hash_map_entry);
}

static hash_map_entry_t *find_bucket_entry_(list_t *hash_bucket_list,
        const void *key)
{

    if (hash_bucket_list == NULL) {
        return NULL;
    }

    for (const list_node_t *iter = osi_list_begin(hash_bucket_list);
            iter != osi_list_end(hash_bucket_list);
            iter = osi_list_next(iter)) {
        hash_map_entry_t *hash_map_entry = (hash_map_entry_t *)osi_list_node(iter);
        if (hash_map_entry->hash_map->keys_are_equal(hash_map_entry->key, key)) {
            return hash_map_entry;
        }
    }
    return NULL;
}

static bool default_key_equality(const void *x, const void *y)
{
    return x == y;
}
