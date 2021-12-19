/* a generic and efficient heap                                             */

/* Copyright (c) 2004-2005, 2007-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt/sysdep.h"
#include "xbt/log.h"
#include "heap_private.h"

#include <stdio.h>
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(xbt_heap, xbt, "Heap");

static void xbt_heap_max_heapify(xbt_heap_t H, int i);
static void xbt_heap_increase_key(xbt_heap_t H, int i);

/** @addtogroup XBT_heap
 *  \brief This section describes the API to generic heap with O(log(n)) access.
 */

/**
 * @brief Creates a new heap.
 * \param init_size initial size of the heap
 * \param free_func function to call on each element when you want to free
 *             the whole heap (or NULL if nothing to do).
 *
 * Creates a new heap.
 */
XBT_INLINE xbt_heap_t xbt_heap_new(int init_size,
                                   void_f_pvoid_t const free_func)
{
  xbt_heap_t H = xbt_new0(struct xbt_heap, 1);
  H->size = init_size;
  H->count = 0;
  H->items = (xbt_heap_item_t) xbt_new0(struct xbt_heap_item, init_size);
  H->free = free_func;
  return H;
}

/**
 * @brief Set the update callback function.
 * @param H the heap we're working on
 * \param update_callback function to call on each element to update its index when needed.
 */
XBT_INLINE void xbt_heap_set_update_callback(xbt_heap_t H,
                                             void (*update_callback) (void
                                                                      *,
                                                                      int))
{
  H->update_callback = update_callback;
}


/**
 * @brief kilkil a heap and its content
 * @param H poor victim
 */
void xbt_heap_free(xbt_heap_t H)
{
  int i;
  if (H->free)
    for (i = 0; i < H->count; i++)
      H->free(H->items[i].content);
  free(H->items);
  free(H);
  return;
}

/**
 * @brief returns the number of elements in the heap
 * @param H the heap we're working on
 * @return the number of elements in the heap
 */
XBT_INLINE int xbt_heap_size(xbt_heap_t H)
{
  return (H->count);
}

/**
 * @brief Add an element into the heap.
 * \param H the heap we're working on
 * \param content the object you want to add to the heap
 * \param key the key associated to this object
 *
 * The element with the smallest key is automatically moved at the top of the heap.
 */
void xbt_heap_push(xbt_heap_t H, void *content, double key)
{
  int count = ++(H->count);

  int size = H->size;
  xbt_heap_item_t item;

  if (count > size) {
    H->size = (size << 1) + 1;
    H->items =
        (void *) xbt_realloc(H->items,
                         (H->size) * sizeof(struct xbt_heap_item));
  }

  item = &(H->items[count - 1]);
  item->key = key;
  item->content = content;
  xbt_heap_increase_key(H, count - 1);
  XBT_DEBUG("Heap has now %d elements and max elem is %g",xbt_heap_size(H),xbt_heap_maxkey(H));
  return;
}


/**
 * @brief Extracts from the heap and returns the element with the smallest key.
 * \param H the heap we're working on
 * \return the element with the smallest key
 *
 * Extracts from the heap and returns the element with the smallest
 * key. The element with the next smallest key is automatically moved
 * at the top of the heap.
 */
void *xbt_heap_pop(xbt_heap_t H)
{
  xbt_heap_item_t items = H->items;
  int size = H->size;
  void *max;

  if (H->count == 0)
    return NULL;

  XBT_DEBUG("Heap has %d elements before extraction and max elem was %g",xbt_heap_size(H),xbt_heap_maxkey(H));

  max = CONTENT(H, 0);

  items[0] = items[(H->count) - 1];
  (H->count)--;
  xbt_heap_max_heapify(H,0);
  if (H->count < size >> 2 && size > 16) {
    size = (size >> 1) + 1;
    H->items =
        (void *) xbt_realloc(items,
                         size * sizeof(struct xbt_heap_item));
    H->size = size;
  }

  if (H->update_callback)
    H->update_callback(max, -1);
  return max;
}

/**
 * @brief Extracts from the heap and returns the element at position i.
 * \param H the heap we're working on
 * \param i  element position
 * \return the element at position i if ok, NULL otherwise
 *
 * Extracts from the heap and returns the element at position i. The heap is automatically reorded.
 */
void *xbt_heap_remove(xbt_heap_t H, int i)
{
  XBT_DEBUG("Heap has %d elements: extracting element %d",xbt_heap_size(H),i);

  if ((i < 0) || (i > H->count - 1))
    return NULL;
  /* put element i at head */
  if (i > 0) {
    KEY(H, i) = MIN_KEY_VALUE;
    xbt_heap_increase_key(H, i);
  }

  return xbt_heap_pop(H);
}

/**
 * @brief Updates an element of the heap with a new value.
 * \param H the heap we're working on
 * \param i  element position
 * \param key new value for the element
 *
 * Updates an element of the heap with a new value.
 */
void xbt_heap_update(xbt_heap_t H, int i, double key)
{
  XBT_DEBUG("Heap has %d elements: updating element %d : was %1.12f to %1.12f ",xbt_heap_size(H),i,KEY(H, i), key);

  if ((i < 0) || (i > H->count - 1) || key == KEY(H, i))
    return ;

  if(key< KEY(H, i)){
    KEY(H, i)=key;
    xbt_heap_increase_key(H, i);
  }else{
    KEY(H, i)=key;
    xbt_heap_max_heapify(H,i);
  }
}

/**
 * @brief returns the smallest key in the heap (heap unchanged)
 * \param H the heap we're working on
 *
 * \return the smallest key in the heap without modifying the heap.
 */
XBT_INLINE double xbt_heap_maxkey(xbt_heap_t H)
{
  xbt_assert(H->count != 0, "Empty heap");
  return KEY(H, 0);
}

/**
 * @brief returns the value associated to the smallest key in the heap (heap unchanged)
 * \param H the heap we're working on
 *
 * \return the value associated to the smallest key in the heap
 * without modifying the heap.
 */
void *xbt_heap_maxcontent(xbt_heap_t H)
{
  xbt_assert(H->count != 0, "Empty heap");
  return CONTENT(H, 0);
}

/* <<<< private >>>>
 * \param H the heap we're working on
 *
 * Restores the heap property once an element has been deleted.
 */
static void xbt_heap_max_heapify(xbt_heap_t H, int index)
{
  int i = index;
  int count = H->count;
  xbt_heap_item_t items = H->items;

  while (1) {
    int greatest = i;
    int l = LEFT(i);
    int r = l + 1;
    if (l < count && items[l].key < items[i].key)
      greatest = l;
    if (r < count && items[r].key < items[greatest].key)
      greatest = r;
    if (greatest != i) {
      struct xbt_heap_item tmp = items[i];
      items[i] = items[greatest];
      items[greatest] = tmp;
      if (H->update_callback)
        H->update_callback(items[i].content, i);
      i = greatest;
    } else {
      if (H->update_callback)
        H->update_callback(items[i].content, i);
      return;
    }
  }
}

/* <<<< private >>>>
 * \param H the heap we're working on
 * \param i an item position in the heap
 *
 * Moves up an item at position i to its correct position. Works only
 * when called from xbt_heap_push. Do not use otherwise.
 */
static void xbt_heap_increase_key(xbt_heap_t H, int i)
{
  xbt_heap_item_t items = H->items;
  int p = PARENT(i);
  while (i > 0 && items[p].key > items[i].key) {
    struct xbt_heap_item tmp = items[i];
    items[i] = items[p];
    items[p] = tmp;
    if (H->update_callback)
      H->update_callback(items[i].content, i);
    i = p;
    p = PARENT(i);
  }
  if (H->update_callback)
    H->update_callback(items[i].content, i);
  return;
}
