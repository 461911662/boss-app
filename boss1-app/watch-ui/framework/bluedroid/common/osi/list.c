/**
 * @file list.c
 * @brief 链表操作文件
 * @attention 可以自由学习
 */

/****************************************************************************
 * INCLUDES
 ****************************************************************************/
#include "bt_common.h"
#include "osi/allocator.h"
#include "osi/list.h"
#include "osi/osi.h"

/****************************************************************************
 * TYPEDEFS
 ****************************************************************************/
struct list_node_t {
    struct list_node_t *next;
    void *data;
};

typedef struct list_t {
    list_node_t *head;
    list_node_t *tail;
    size_t length;
    list_free_cb free_cb;
} list_t;

/****************************************************************************
 * GLOBAL PROTOTYPES
 ****************************************************************************/

// Hidden constructor, only to be used by the hash map for the allocation tracker.
// Behaves the same as |osi_list_new|, except you get to specify the allocator.
static list_t *list_new_internal(list_free_cb callback)
{
    list_t *list = (list_t *) osi_calloc(sizeof(list_t));
    if (!list) {
        return NULL;
    }

    list->head = list->tail = NULL;
    list->length = 0;
    list->free_cb = callback;
    return list;
}

list_t *osi_list_new(list_free_cb callback)
{
    return list_new_internal(callback);
}

void osi_list_free(list_t *list)
{
    if (!list) {
        return;
    }

    osi_list_clear(list);
    osi_free(list);
}

bool osi_list_is_empty(const list_t *list)
{
    assert(list != NULL);
    return (list->length == 0);
}

bool osi_list_contains(const list_t *list, const void *data)
{
  assert(list != NULL);
  assert(data != NULL);

  for (const list_node_t *node = osi_list_begin(list); node != osi_list_end(list); node = osi_list_next(node)) {
    if (osi_list_node(node) == data) {
      return true;
    }
  }

  return false;
}

list_node_t *osi_list_get_node(const list_t *list, const void *data)
{
  assert(list != NULL);
  assert(data != NULL);
  list_node_t *p_node_ret = NULL;
  for (list_node_t *node = osi_list_begin(list); node != osi_list_end(list); node = osi_list_next(node)) {
    if (osi_list_node(node) == data) {
      p_node_ret = node;
      break;
    }
  }

  return p_node_ret;
}

size_t osi_list_length(const list_t *list)
{
    assert(list != NULL);
    return list->length;
}

void *osi_list_front(const list_t *list)
{
    assert(list != NULL);
    assert(!osi_list_is_empty(list));

    return list->head->data;
}

void *osi_list_back(const list_t *list) {
  assert(list != NULL);
  assert(!osi_list_is_empty(list));

  return list->tail->data;
}

list_node_t *osi_list_back_node(const list_t *list) {
  assert(list != NULL);
  assert(!osi_list_is_empty(list));

  return list->tail;
}

bool osi_list_insert_after(list_t *list, list_node_t *prev_node, void *data) {
    assert(list != NULL);
    assert(prev_node != NULL);
    assert(data != NULL);
    list_node_t *node = (list_node_t *)osi_calloc(sizeof(list_node_t));
    if (!node) {
        OSI_TRACE_ERROR("%s osi_calloc failed.\n", __FUNCTION__ );
        return false;
    }
    node->next = prev_node->next;
    node->data = data;
    prev_node->next = node;
    if (list->tail == prev_node) {
        list->tail = node;
    }
    ++list->length;
    return true;
}

bool osi_list_prepend(list_t *list, void *data)
{
    assert(list != NULL);
    assert(data != NULL);
    list_node_t *node = (list_node_t *)osi_calloc(sizeof(list_node_t));
    if (!node) {
        OSI_TRACE_ERROR("%s osi_calloc failed.\n", __FUNCTION__ );
        return false;
    }
    node->next = list->head;
    node->data = data;
    list->head = node;
    if (list->tail == NULL) {
        list->tail = list->head;
    }
    ++list->length;
    return true;
}

bool osi_list_append(list_t *list, void *data)
{
    assert(list != NULL);
    assert(data != NULL);
    list_node_t *node = (list_node_t *)osi_calloc(sizeof(list_node_t));
    if (!node) {
        OSI_TRACE_ERROR("%s osi_calloc failed.\n", __FUNCTION__ );
        return false;
    }
    node->next = NULL;
    node->data = data;
    if (list->tail == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    ++list->length;
    return true;
}

bool osi_list_remove(list_t *list, void *data)
{
    assert(list != NULL);
    assert(data != NULL);

    if (osi_list_is_empty(list)) {
        return false;
    }

    if (list->head->data == data) {
        list_node_t *next = osi_list_free_node(list, list->head);
        if (list->tail == list->head) {
            list->tail = next;
        }
        list->head = next;
        return true;
    }

    for (list_node_t *prev = list->head, *node = list->head->next; node; prev = node, node = node->next)
        if (node->data == data) {
            prev->next = osi_list_free_node(list, node);
            if (list->tail == node) {
                list->tail = prev;
            }
            return true;
        }

    return false;
}

bool osi_list_delete(list_t *list, void *data)
{
    assert(list != NULL);
    assert(data != NULL);

    if (osi_list_is_empty(list)) {
        return false;
    }

    if (list->head->data == data) {
        list_node_t *next = osi_list_delete_node(list, list->head);
        if (list->tail == list->head) {
            list->tail = next;
        }
        list->head = next;
        return true;
    }

    for (list_node_t *prev = list->head, *node = list->head->next; node; prev = node, node = node->next)
        if (node->data == data) {
            prev->next = osi_list_delete_node(list, node);
            if (list->tail == node) {
                list->tail = prev;
            }
            return true;
        }

    return false;
}

void osi_list_clear(list_t *list)
{
    assert(list != NULL);
    for (list_node_t *node = list->head; node; ) {
        node = osi_list_free_node(list, node);
    }
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}

list_node_t *osi_list_foreach(const list_t *list, list_iter_cb callback, void *context)
{
  assert(list != NULL);
  assert(callback != NULL);

  for (list_node_t *node = list->head; node; ) {
    list_node_t *next = node->next;
    if (!callback(node->data, context)) {
      return node;
    }
    node = next;
  }
  return NULL;
}

list_node_t *osi_list_begin(const list_t *list)
{
    assert(list != NULL);
    return list->head;
}

list_node_t *osi_list_end(UNUSED_ATTR const list_t *list)
{
    assert(list != NULL);
    return NULL;
}

list_node_t *osi_list_next(const list_node_t *node)
{
    assert(node != NULL);
    return node->next;
}

void *osi_list_node(const list_node_t *node)
{
    assert(node != NULL);
    return node->data;
}

list_node_t *osi_list_free_node(list_t *list, list_node_t *node)
{
    assert(list != NULL);
    assert(node != NULL);

    list_node_t *next = node->next;

    if (list->free_cb) {
        list->free_cb(node->data);
    }
    osi_free(node);
    --list->length;

    return next;
}

// remove the element from list but do not free the node data
list_node_t *osi_list_delete_node(list_t *list, list_node_t *node)
{
    assert(list != NULL);
    assert(node != NULL);

    list_node_t *next = node->next;

    osi_free(node);
    --list->length;

    return next;
}
