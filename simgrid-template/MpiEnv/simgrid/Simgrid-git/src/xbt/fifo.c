/* Copyright (c) 2004-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt/sysdep.h"
#include "xbt/log.h"
#include "xbt/mallocator.h"
#include "fifo_private.h"
#include "xbt_modinter.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(xbt_fifo, xbt, "FIFO");

static void *fifo_item_mallocator_new_f(void);
#define fifo_item_mallocator_free_f xbt_free_f
static void fifo_item_mallocator_reset_f(void *item);

static xbt_mallocator_t item_mallocator = NULL;

/** Constructor
 * \return a new fifo
 */
xbt_fifo_t xbt_fifo_new(void)
{
  xbt_fifo_t fifo;
  fifo = xbt_new0(struct xbt_fifo, 1);

  return fifo;
}


/** Destructor
 * \param l poor victim
 *
 * Free the fifo structure. None of the objects that was in the fifo is however modified.
 */
void xbt_fifo_free(xbt_fifo_t l)
{
  xbt_fifo_reset(l);
  xbt_free(l);
}

/**
 * \brief Makes a fifo empty.
 * \param l a fifo
 *
 * None of the objects that was in the fifo is however modified.
 */
void xbt_fifo_reset(xbt_fifo_t l)
{
  xbt_fifo_item_t b, tmp;

  for (b = xbt_fifo_get_first_item(l); b;
       tmp = b, b = b->next, xbt_fifo_free_item(tmp));
  l->head = l->tail = NULL;
}

/** Push
 * \param l list
 * \param t element
 * \return the bucket that was just added
 *
 * Add an object at the tail of the list
 */
xbt_fifo_item_t xbt_fifo_push(xbt_fifo_t l, void *t)
{
  xbt_fifo_item_t new;

  new = xbt_fifo_new_item();
  new->content = t;

  xbt_fifo_push_item(l, new);
  return new;
}

/** Pop
 * \param l list
 * \returns the object stored at the tail of the list.
 *
 * Removes and returns the object stored at the tail of the list.
 * Returns NULL if the list is empty.
 */
void *xbt_fifo_pop(xbt_fifo_t l)
{
  xbt_fifo_item_t item;
  void *content;

  if (l == NULL)
    return NULL;
  if (!(item = xbt_fifo_pop_item(l)))
    return NULL;

  content = item->content;
  xbt_fifo_free_item(item);
  return content;
}

/**
 * \param l list
 * \param t element
 * \return the bucket that was just added
 *
 * Add an object at the head of the list
 */
xbt_fifo_item_t xbt_fifo_unshift(xbt_fifo_t l, void *t)
{
  xbt_fifo_item_t new;

  new = xbt_fifo_new_item();
  new->content = t;
  xbt_fifo_unshift_item(l, new);
  return new;
}

/** Shift
 * \param l list
 * \returns the object stored at the head of the list.
 *
 * Removes and returns the object stored at the head of the list.
 * Returns NULL if the list is empty.
 */
void *xbt_fifo_shift(xbt_fifo_t l)
{
  xbt_fifo_item_t item;
  void *content;

  if (l == NULL)
    return NULL;
  if (!(item = xbt_fifo_shift_item(l)))
    return NULL;

  content = item->content;
  xbt_fifo_free_item(item);
  return content;
}

/** Push a bucket
 * \param l list
 * \param new bucket
 *
 * Hook up this bucket at the tail of the list
 */
void xbt_fifo_push_item(xbt_fifo_t l, xbt_fifo_item_t new)
{
  xbt_assert((new->next == NULL) && (new->prev == NULL), "Invalid item!");
  (l->count)++;
  if (l->head == NULL) {
    l->head = new;
    l->tail = new;
    return;
  }
  new->prev = l->tail;
  new->prev->next = new;
  l->tail = new;
}

/** Pop bucket
 * \param l
 * \returns the bucket that was at the tail of the list.
 *
 * Returns NULL if the list was empty.
 */
xbt_fifo_item_t xbt_fifo_pop_item(xbt_fifo_t l)
{
  xbt_fifo_item_t item;

  if (l->tail == NULL)
    return NULL;

  item = l->tail;

  l->tail = item->prev;
  if (l->tail == NULL)
    l->head = NULL;
  else
    l->tail->next = NULL;

  (l->count)--;

  item->prev = NULL;

  return item;
}

/** Push a bucket
 * \param l list
 * \param new bucket
 *
 * Hook up this bucket at the head of the list
 */
void xbt_fifo_unshift_item(xbt_fifo_t l, xbt_fifo_item_t new)
{
  xbt_assert((new->next == NULL) && (new->prev == NULL), "Invalid item!");
  (l->count)++;
  if (l->head == NULL) {
    l->head = new;
    l->tail = new;
    return;
  }
  new->next = l->head;
  new->next->prev = new;
  l->head = new;
  return;
}

/** Shift bucket
 * \param l
 * \returns the bucket that was at the head of the list.
 *
 * Returns NULL if the list was empty.
 */
xbt_fifo_item_t xbt_fifo_shift_item(xbt_fifo_t l)
{
  xbt_fifo_item_t item;

  if (l->head == NULL)
    return NULL;

  item = l->head;

  l->head = item->next;
  if (l->head == NULL)
    l->tail = NULL;
  else
    l->head->prev = NULL;

  (l->count)--;

  item->next = NULL;

  return item;
}

/**
 * \param l
 * \param t an objet
 *
 * removes the first occurence of \a t from \a l.
 * \warning it will not remove duplicates
 * \return 1 if an item was removed and 0 otherwise.
 */
int xbt_fifo_remove(xbt_fifo_t l, void *t)
{
  xbt_fifo_item_t current, current_next;


  for (current = l->head; current; current = current_next) {
    current_next = current->next;
    if (current->content != t)
      continue;
    /* remove the item */
    xbt_fifo_remove_item(l, current);
    xbt_fifo_free_item(current);
    /* WILL NOT REMOVE DUPLICATES */
    return 1;
  }
  return 0;
}


/**
 * \param l
 * \param t an objet
 *
 * removes all occurences of \a t from \a l.
 * \return 1 if an item was removed and 0 otherwise.
 */
int xbt_fifo_remove_all(xbt_fifo_t l, void *t)
{
  xbt_fifo_item_t current, current_next;
  int res = 0;

  for (current = l->head; current; current = current_next) {
    current_next = current->next;
    if (current->content != t)
      continue;
    /* remove the item */
    xbt_fifo_remove_item(l, current);
    xbt_fifo_free_item(current);
    res = 1;
  }
  return res;
}

/**
 * \param l a list
 * \param current a bucket
 *
 * removes a bucket \a current from the list \a l. This function implicitely
 * assumes (and doesn't check!) that this item belongs to this list...
 */
void xbt_fifo_remove_item(xbt_fifo_t l, xbt_fifo_item_t current)
{
  if (l->head == l->tail) {     /* special case */
    xbt_assert((current == l->head), "This item is not in the list!");
    l->head = NULL;
    l->tail = NULL;
    (l->count)--;
    current->prev = current->next = NULL;
    return;
  }

  if (current == l->head) {     /* It's the head */
    l->head = current->next;
    l->head->prev = NULL;
  } else if (current == l->tail) {      /* It's the tail */
    l->tail = current->prev;
    l->tail->next = NULL;
  } else {                      /* It's in the middle */
    current->prev->next = current->next;
    current->next->prev = current->prev;
  }
  (l->count)--;
  current->prev = current->next = NULL;
}

/**
 * \param f a list
 * \param content an object
 * \return 1 if \a content is in \a f.
 */
int xbt_fifo_is_in(xbt_fifo_t f, void *content)
{
  xbt_fifo_item_t item = xbt_fifo_get_first_item(f);
  while (item) {
    if (item->content == content)
      return 1;
    item = item->next;
  }
  return 0;
}

/**
 * @brief Search the given element in the fifo using a comparison function
 *
 * This function allows to search an item with a user provided function instead
 * of the pointer comparison used elsewhere in this module. Assume for example that you have a fifo of
 * strings. You cannot use xbt_fifo_remove() to remove, say, "TOTO" from it because internally, xbt_fifo_remove()
 * will do something like "if (item->content == "toto"), then remove it". And the pointer to the item content
 * and the pointer to "toto" will never match. As a solution, the current function provides a way to search elements
 * that are semanticaly equivalent instead of only syntaxically. So, removing "Toto" from a fifo can be
 * achieved this way:
 *
 *  @verbatim
int my_comparison_function(void *searched, void *seen) {
  return !strcmp(searched, seen);
}

  xbt_fifo_remove_item(fifo,
                       xbt_fifo_search_item(fifo, my_comparison_function, "Toto"));
@endverbatim
 *
 * \param f a fifo list
 * \param cmp_fun the comparison function. Prototype: void *a,void *b -> int. Semantic: returns true iff a=b
 * @param closure the element to search. It will be provided as first argument to each call of cmp_fun
 * \return the first item matching the comparison function, or NULL if no such item exists
 */
xbt_fifo_item_t xbt_fifo_search_item(xbt_fifo_t f, int_f_pvoid_pvoid_t cmp_fun, void *closure) {
  xbt_fifo_item_t item = xbt_fifo_get_first_item(f);
  while (item) {
    if (cmp_fun(closure, item->content))
      return item;
    item = item->next;
  }
  return NULL;

}

/**
 * \param f a list
 * \return a table with the objects stored in \a f.
 */
void **xbt_fifo_to_array(xbt_fifo_t f)
{
  void **array;
  xbt_fifo_item_t b;
  int i;

  if (f->count == 0)
    return NULL;
  else
    array = xbt_new0(void *, f->count);

  for (i = 0, b = xbt_fifo_get_first_item(f); b; i++, b = b->next) {
    array[i] = b->content;
  }
  return array;
}

/**
 * \param f a list
 * \return a copy of \a f.
 */
xbt_fifo_t xbt_fifo_copy(xbt_fifo_t f)
{
  xbt_fifo_t copy = NULL;
  xbt_fifo_item_t b;

  copy = xbt_fifo_new();

  for (b = xbt_fifo_get_first_item(f); b; b = b->next) {
    xbt_fifo_push(copy, b->content);
  }
  return copy;
}

/* Functions passed to the mallocator constructor */
static void *fifo_item_mallocator_new_f(void)
{
  return xbt_new(s_xbt_fifo_item_t, 1);
}

static void fifo_item_mallocator_reset_f(void *item)
{
  /* memset to zero like calloc */
  memset(item, 0, sizeof(s_xbt_fifo_item_t));
}

/** Constructor
 * \return a new bucket
 */
XBT_INLINE xbt_fifo_item_t xbt_fifo_new_item(void)
{
  return xbt_mallocator_get(item_mallocator);
}

/** \deprecated Use #xbt_fifo_new_item instead.
 */
XBT_INLINE xbt_fifo_item_t xbt_fifo_newitem(void)
{
  XBT_CWARN(xbt_fifo, "This function is deprecated. Use xbt_fifo_new_item.");
  return xbt_fifo_new_item();
}

/**
 * \param i a bucket
 * \param v an object
 *
 * stores \a v in \a i.
 */
XBT_INLINE void xbt_fifo_set_item_content(xbt_fifo_item_t i, void *v)
{
  xbt_fifo_setItemcontent(i, v);
}

/**
 * \param i a bucket
 * \return the object stored \a i.
 */
XBT_INLINE void *xbt_fifo_get_item_content(xbt_fifo_item_t i)
{
  return xbt_fifo_getItemcontent(i);
}

/** Destructor
 * \param b poor victim
 *
 * Free the bucket but does not modifies the object (if any) that was stored in it.
 */
XBT_INLINE void xbt_fifo_free_item(xbt_fifo_item_t b)
{
  xbt_mallocator_release(item_mallocator, b);
  return;
}

/** Destructor
 * \deprecated Use #xbt_fifo_free_item instead.
 */
XBT_INLINE void xbt_fifo_freeitem(xbt_fifo_item_t b)
{
  XBT_CWARN(xbt_fifo, "This function is deprecated. Use xbt_fifo_free_item.");
  xbt_fifo_free_item(b);
  return;
}

/**
 * \param f a list
 * \return the number of buckets in \a f.
 */
XBT_INLINE int xbt_fifo_size(xbt_fifo_t f)
{
  return f->count;
}

/**
 * \param l a list
 * \return the head of \a l.
 *
 * Returns NULL if the list is empty.
 */
XBT_INLINE xbt_fifo_item_t xbt_fifo_get_first_item(xbt_fifo_t l)
{
  return l->head;
}

/**
 * \param l a list
 * \return the tail of \a l.
 *
 * Returns NULL if the list is empty.
 */
XBT_INLINE xbt_fifo_item_t xbt_fifo_get_last_item(xbt_fifo_t l)
{
  return l->tail;
}

/** \deprecated Use #xbt_fifo_get_first_item instead.
 */
XBT_INLINE xbt_fifo_item_t xbt_fifo_getFirstItem(xbt_fifo_t l)
{
  XBT_CWARN(xbt_fifo, "This function is deprecated. Use xbt_fifo_get_first_item.");
  return xbt_fifo_get_first_item(l);
}

/**
 * \param i a bucket
 * \return the bucket that comes next
 *
 * Returns NULL if \a i is the tail of the list.
 */
XBT_INLINE xbt_fifo_item_t xbt_fifo_get_next_item(xbt_fifo_item_t i)
{
  if (i)
    return i->next;
  return NULL;
}

/** \deprecated Use #xbt_fifo_get_next_item instead.
 */
xbt_fifo_item_t xbt_fifo_getNextItem(xbt_fifo_item_t i)
{
  XBT_CWARN(xbt_fifo, "This function is deprecated. Use xbt_fifo_get_next_item.");
  return xbt_fifo_get_next_item(i);
}

/**
 * \param i a bucket
 * \return the bucket that is just before \a i.
 *
 * Returns NULL if \a i is the head of the list.
 */
XBT_INLINE xbt_fifo_item_t xbt_fifo_get_prev_item(xbt_fifo_item_t i)
{
  if (i)
    return i->prev;
  return NULL;
}

/** \deprecated Use #xbt_fifo_get_prev_item instead.
 */
xbt_fifo_item_t xbt_fifo_getPrevItem(xbt_fifo_item_t i)
{
  XBT_WARN("This function is deprecated. Use xbt_fifo_get_prev_item.");
  return xbt_fifo_get_prev_item(i);
}

/* Module init/exit handling the fifo item mallocator
 * These are internal XBT functions called by xbt_preinit/postexit().
 * It can be used several times to recreate the mallocator, for example when you switch to MC mode
 */
void xbt_fifo_preinit(void)
{
  item_mallocator = xbt_mallocator_new(65536,
                                       fifo_item_mallocator_new_f,
                                       fifo_item_mallocator_free_f,
                                       fifo_item_mallocator_reset_f);
}

void xbt_fifo_postexit(void)
{
  if (item_mallocator != NULL) {
    xbt_mallocator_free(item_mallocator);
    item_mallocator = NULL;
  }
}

/* @} */
