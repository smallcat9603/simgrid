/* dict - a generic dictionary, variation over hash table                   */

/* Copyright (c) 2004-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <string.h>
#include <stdio.h>
#include "xbt/ex.h"
#include "xbt/log.h"
#include "xbt/mallocator.h"
#include "xbt_modinter.h"
#include "xbt/str.h"
#include "dict_private.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(xbt_dict, xbt,
                                "Dictionaries provide the same functionalities than hash tables");

/**
 * \brief Constructor
 * \return pointer to the destination
 * \see xbt_dict_free()
 *
 * Creates and initialize a new dictionary with a default hashtable size.
 * The dictionary is heterogeneous: each element can have a different free
 * function.
 */
xbt_dict_t xbt_dict_new(void)
{
  xbt_dict_t dict = xbt_dict_new_homogeneous(NULL);
  dict->homogeneous = 0;

  return dict;
}

/**
 * \brief Constructor
 * \param free_ctn function to call with (\a data as argument) when
 *        \a data is removed from the dictionary
 * \return pointer to the destination
 * \see xbt_dict_new(), xbt_dict_free()
 *
 * Creates and initialize a new dictionary with a default hashtable size.
 * The dictionary is homogeneous: each element share the same free function.
 */
xbt_dict_t xbt_dict_new_homogeneous(void_f_pvoid_t free_ctn)
{
  xbt_dict_t dict;

  dict = xbt_new(s_xbt_dict_t, 1);
  dict->free_f = free_ctn;
  dict->table_size = 127;
  dict->table = xbt_new0(xbt_dictelm_t, dict->table_size + 1);
  dict->count = 0;
  dict->fill = 0;
  dict->homogeneous = 1;

  return dict;
}

/**
 * \brief Destructor
 * \param dict the dictionary to be freed
 *
 * Frees a dictionary with all the data
 */
void xbt_dict_free(xbt_dict_t * dict)
{
  int i;
  xbt_dictelm_t current, previous;
  int table_size;
  xbt_dictelm_t *table;

  //  if ( *dict )  xbt_dict_dump_sizes(*dict);

  if (dict != NULL && *dict != NULL) {
    table_size = (*dict)->table_size;
    table = (*dict)->table;
    /* Warning: the size of the table is 'table_size+1'...
     * This is because table_size is used as a binary mask in xbt_dict_rehash */
    for (i = 0; (*dict)->count && i <= table_size; i++) {
      current = table[i];
      while (current != NULL) {
        previous = current;
        current = current->next;
        xbt_dictelm_free(*dict, previous);
        (*dict)->count--;
      }
    }
    xbt_free(table);
    xbt_free(*dict);
    *dict = NULL;
  }
}

/**
 * Returns the amount of elements in the dict
 */
XBT_INLINE unsigned int xbt_dict_size(xbt_dict_t dict)
{
  return (dict ? (unsigned int) dict->count : (unsigned int) 0);
}

/* Expend the size of the dict */
static void xbt_dict_rehash(xbt_dict_t dict)
{
  const int oldsize = dict->table_size + 1;
  register int newsize = oldsize * 2;
  register int i;
  register xbt_dictelm_t *currcell;
  register xbt_dictelm_t *twincell;
  register xbt_dictelm_t bucklet;
  register xbt_dictelm_t *pprev;

  currcell =
      (xbt_dictelm_t *) xbt_realloc((char *) dict->table,
                                    newsize * sizeof(xbt_dictelm_t));
  memset(&currcell[oldsize], 0, oldsize * sizeof(xbt_dictelm_t));       /* zero second half */
  dict->table_size = --newsize;
  dict->table = currcell;
  XBT_DEBUG("REHASH (%d->%d)", oldsize, newsize);

  for (i = 0; i < oldsize; i++, currcell++) {
    if (!*currcell)             /* empty cell */
      continue;
    twincell = currcell + oldsize;
    for (pprev = currcell, bucklet = *currcell; bucklet; bucklet = *pprev) {
      /* Since we use "& size" instead of "%size" and since the size was doubled,
         each bucklet of this cell must either :
         - stay  in  cell i (ie, currcell)
         - go to the cell i+oldsize (ie, twincell) */
      if ((bucklet->hash_code & newsize) != i) {        /* Move to b */
        *pprev = bucklet->next;
        bucklet->next = *twincell;
        if (!*twincell)
          dict->fill++;
        *twincell = bucklet;
        continue;
      } else {
        pprev = &bucklet->next;
      }

    }

    if (!*currcell)             /* everything moved */
      dict->fill--;
  }
}

/**
 * \brief Add data to the dict (arbitrary key)
 * \param dict the container
 * \param key the key to set the new data
 * \param key_len the size of the \a key
 * \param data the data to add in the dict
 * \param free_ctn function to call with (\a data as argument) when
 *        \a data is removed from the dictionary
 *
 * Set the \a data in the structure under the \a key, which can be any kind
 * of data, as long as its length is provided in \a key_len.
 */
XBT_INLINE void xbt_dict_set_ext(xbt_dict_t dict,
                                 const char *key, int key_len,
                                 void *data, void_f_pvoid_t free_ctn)
{

  unsigned int hash_code = xbt_str_hash_ext(key, key_len);

  xbt_dictelm_t current, previous = NULL;
  xbt_assert(dict);

  XBT_CDEBUG(xbt_dict,
             "ADD %.*s hash = %u, size = %d, & = %u", key_len, key, hash_code,
             dict->table_size, hash_code & dict->table_size);
  current = dict->table[hash_code & dict->table_size];
  while (current != NULL &&
         (hash_code != current->hash_code || key_len != current->key_len
          || memcmp(key, current->key, key_len))) {
    previous = current;
    current = current->next;
  }

  if (current == NULL) {
    /* this key doesn't exist yet */
    current = xbt_dictelm_new(dict, key, key_len, hash_code, data, free_ctn);
    dict->count++;
    if (previous == NULL) {
      dict->table[hash_code & dict->table_size] = current;
      dict->fill++;
      if ((dict->fill * 100) / (dict->table_size + 1) > MAX_FILL_PERCENT)
        xbt_dict_rehash(dict);
    } else {
      previous->next = current;
    }
  } else {
    XBT_CDEBUG(xbt_dict, "Replace %.*s by %.*s under key %.*s",
               key_len, (char *) current->content,
               key_len, (char *) data, key_len, (char *) key);
    /* there is already an element with the same key: overwrite it */
    xbt_dictelm_set_data(dict, current, data, free_ctn);
  }
}

/**
 * \brief Add data to the dict (null-terminated key)
 *
 * \param dict the dict
 * \param key the key to set the new data
 * \param data the data to add in the dict
 * \param free_ctn function to call with (\a data as argument) when
 *        \a data is removed from the dictionary
 *
 * set the \a data in the structure under the \a key, which is a
 * null terminated string.
 */
XBT_INLINE void xbt_dict_set(xbt_dict_t dict,
                             const char *key, void *data,
                             void_f_pvoid_t free_ctn)
{

  xbt_dict_set_ext(dict, key, strlen(key), data, free_ctn);
}

/**
 * \brief Retrieve data from the dict (arbitrary key)
 *
 * \param dict the dealer of data
 * \param key the key to find data
 * \param key_len the size of the \a key
 * \return the data that we are looking for
 *
 * Search the given \a key. Throws not_found_error when not found.
 */
XBT_INLINE void *xbt_dict_get_ext(xbt_dict_t dict, const char *key,
                                  int key_len)
{


  unsigned int hash_code = xbt_str_hash_ext(key, key_len);
  xbt_dictelm_t current;

  xbt_assert(dict);

  current = dict->table[hash_code & dict->table_size];
  while (current != NULL &&
         (hash_code != current->hash_code || key_len != current->key_len
          || memcmp(key, current->key, key_len))) {
    current = current->next;
  }

  if (current == NULL)
    THROWF(not_found_error, 0, "key %.*s not found", key_len, key);

  return current->content;
}

/**
 * \brief like xbt_dict_get_ext(), but returning NULL when not found
 */
void *xbt_dict_get_or_null_ext(xbt_dict_t dict, const char *key,
                               int key_len)
{

  unsigned int hash_code = xbt_str_hash_ext(key, key_len);
  xbt_dictelm_t current;

  xbt_assert(dict);

  current = dict->table[hash_code & dict->table_size];
  while (current != NULL &&
         (hash_code != current->hash_code || key_len != current->key_len
          || memcmp(key, current->key, key_len))) {
    current = current->next;
  }

  if (current == NULL)
    return NULL;

  return current->content;
}

/**
 * @brief retrieve the key associated to that object. Warning, that's a linear search
 *
 * Returns NULL if the object cannot be found
 */
char *xbt_dict_get_key(xbt_dict_t dict, const void *data)
{
  int i;
  xbt_dictelm_t current;


  for (i = 0; i <= dict->table_size; i++) {
    current = dict->table[i];
    while (current != NULL) {
      if (current->content == data)
        return current->key;
      current = current->next;
    }
  }

  return NULL;
}

/**
 * @brief retrieve the key associated to that xbt_dictelm_t.
 *
 */
char *xbt_dict_get_elm_key(xbt_dictelm_t elm)
{
  return elm->key;
}

/**
 * \brief Retrieve data from the dict (null-terminated key)
 *
 * \param dict the dealer of data
 * \param key the key to find data
 * \return the data that we are looking for
 *
 * Search the given \a key. Throws not_found_error when not found.
 * Check xbt_dict_get_or_null() for a version returning NULL without exception when
 * not found.
 */
XBT_INLINE void *xbt_dict_get(xbt_dict_t dict, const char *key)
{
  return xbt_dict_get_elm(dict, key)->content;
}
/**
 * \brief Retrieve element from the dict (null-terminated key)
 *
 * \param dict the dealer of data
 * \param key the key to find data
 * \return the s_xbt_dictelm_t that we are looking for
 *
 * Search the given \a key. Throws not_found_error when not found.
 * Check xbt_dict_get_or_null() for a version returning NULL without exception when
 * not found.
 */
XBT_INLINE xbt_dictelm_t xbt_dict_get_elm(xbt_dict_t dict, const char *key)
{
  xbt_dictelm_t current = xbt_dict_get_elm_or_null(dict, key);

  if (current == NULL)
    THROWF(not_found_error, 0, "key %s not found", key);

  return current;
}

/**
 * \brief like xbt_dict_get(), but returning NULL when not found
 */
XBT_INLINE void *xbt_dict_get_or_null(xbt_dict_t dict, const char *key)
{
  xbt_dictelm_t current = xbt_dict_get_elm_or_null(dict, key);

  if (current == NULL)
    return NULL;

  return current->content;
}
/**
 * \brief like xbt_dict_get_elm(), but returning NULL when not found
 */
XBT_INLINE xbt_dictelm_t xbt_dict_get_elm_or_null(xbt_dict_t dict, const char *key)
{
  unsigned int hash_code = xbt_str_hash(key);
  xbt_dictelm_t current;

  xbt_assert(dict);

  current = dict->table[hash_code & dict->table_size];
  while (current != NULL &&
         (hash_code != current->hash_code || strcmp(key, current->key)))
    current = current->next;
  return current;
}


/**
 * \brief Remove data from the dict (arbitrary key)
 *
 * \param dict the trash can
 * \param key the key of the data to be removed
 * \param key_len the size of the \a key
 *
 * Remove the entry associated with the given \a key (throws not_found)
 */
XBT_INLINE void xbt_dict_remove_ext(xbt_dict_t dict, const char *key,
                                    int key_len)
{


  unsigned int hash_code = xbt_str_hash_ext(key, key_len);
  xbt_dictelm_t current, previous = NULL;

  xbt_assert(dict);

  //  fprintf(stderr,"RM %.*s hash = %d, size = %d, & = %d\n",key_len,key,hash_code, dict->table_size, hash_code & dict->table_size);
  current = dict->table[hash_code & dict->table_size];
  while (current != NULL &&
         (hash_code != current->hash_code || key_len != current->key_len
          || strncmp(key, current->key, key_len))) {
    previous = current;         /* save the previous node */
    current = current->next;
  }

  if (current == NULL)
    THROWF(not_found_error, 0, "key %.*s not found", key_len, key);

  if (previous != NULL) {
    previous->next = current->next;
  } else {
    dict->table[hash_code & dict->table_size] = current->next;
  }

  if (!dict->table[hash_code & dict->table_size])
    dict->fill--;

  xbt_dictelm_free(dict, current);
  dict->count--;
}



/**
 * \brief Remove data from the dict (null-terminated key)
 *
 * \param dict the dict
 * \param key the key of the data to be removed
 *
 * Remove the entry associated with the given \a key
 */
XBT_INLINE void xbt_dict_remove(xbt_dict_t dict, const char *key)
{
  xbt_dict_remove_ext(dict, key, strlen(key));
}

#ifdef XBT_USE_DEPRECATED
/**
 * \brief Add data to the dict (arbitrary key)
 * \param dict the container
 * \param key the key to set the new data
 * \param data the data to add in the dict
 *
 * Set the \a data in the structure under the \a key.
 * Both \a data and \a key are considered as uintptr_t.
 */
XBT_INLINE void xbt_dicti_set(xbt_dict_t dict,
                              uintptr_t key, uintptr_t data)
{
  xbt_dict_set_ext(dict, (void *)&key, sizeof key, (void*)data, NULL);
}

/**
 * \brief Retrieve data from the dict (key considered as a uintptr_t)
 *
 * \param dict the dealer of data
 * \param key the key to find data
 * \return the data that we are looking for (or 0 if not found)
 *
 * Mixing uintptr_t keys with regular keys in the same dict is discouraged
 */
XBT_INLINE uintptr_t xbt_dicti_get(xbt_dict_t dict, uintptr_t key)
{
  return (uintptr_t)xbt_dict_get_or_null_ext(dict, (void *)&key, sizeof key);
}

/** Remove a uintptr_t key from the dict */
XBT_INLINE void xbt_dicti_remove(xbt_dict_t dict, uintptr_t key)
{
  xbt_dict_remove_ext(dict, (void *)&key, sizeof key);
}
#endif

/**
 * \brief Remove all data from the dict
 * \param dict the dict
 */
void xbt_dict_reset(xbt_dict_t dict)
{

  int i;
  xbt_dictelm_t current, previous = NULL;

  xbt_assert(dict);

  if (dict->count == 0)
    return;

  for (i = 0; i <= dict->table_size; i++) {
    current = dict->table[i];
    while (current != NULL) {
      previous = current;
      current = current->next;
      xbt_dictelm_free(dict, previous);
    }
    dict->table[i] = NULL;
  }

  dict->count = 0;
  dict->fill = 0;
}

/**
 * \brief Return the number of elements in the dict.
 * \param dict a dictionary
 */
XBT_INLINE int xbt_dict_length(xbt_dict_t dict)
{
  xbt_assert(dict);

  return dict->count;
}

/** @brief function to be used in xbt_dict_dump as long as the stored values are strings */
void xbt_dict_dump_output_string(void *s)
{
  fputs(s, stdout);
}

/**
 * \brief test if the dict is empty or not
 */
XBT_INLINE int xbt_dict_is_empty(xbt_dict_t dict)
{
  return !dict || (xbt_dict_length(dict) == 0);
}

/**
 * \brief Outputs the content of the structure (debugging purpose)
 *
 * \param dict the exibitionist
 * \param output a function to dump each data in the tree (check @ref xbt_dict_dump_output_string)
 *
 * Outputs the content of the structure. (for debugging purpose). \a output is a
 * function to output the data. If NULL, data won't be displayed.
 */

void xbt_dict_dump(xbt_dict_t dict, void_f_pvoid_t output)
{
  int i;
  xbt_dictelm_t element;
  printf("Dict %p:\n", dict);
  if (dict != NULL) {
    for (i = 0; i < dict->table_size; i++) {
      element = dict->table[i];
      if (element) {
        printf("[\n");
        while (element != NULL) {
          printf(" %s -> '", element->key);
          if (output != NULL) {
            output(element->content);
          }
          printf("'\n");
          element = element->next;
        }
        printf("]\n");
      } else {
        printf("[]\n");
      }
    }
  }
}

xbt_dynar_t all_sizes = NULL;
/** @brief shows some debugging info about the bucklet repartition */
void xbt_dict_dump_sizes(xbt_dict_t dict)
{

  int i;
  unsigned int count;
  unsigned int size;
  xbt_dictelm_t element;
  xbt_dynar_t sizes = xbt_dynar_new(sizeof(int), NULL);

  printf("Dict %p: %d bucklets, %d used cells (of %d) ", dict, dict->count,
         dict->fill, dict->table_size);
  if (dict != NULL) {
    for (i = 0; i < dict->table_size; i++) {
      element = dict->table[i];
      size = 0;
      if (element) {
        while (element != NULL) {
          size++;
          element = element->next;
        }
      }
      if (xbt_dynar_length(sizes) <= size) {
        int prevsize = 1;
        xbt_dynar_set(sizes, size, &prevsize);
      } else {
        int prevsize;
        xbt_dynar_get_cpy(sizes, size, &prevsize);
        prevsize++;
        xbt_dynar_set(sizes, size, &prevsize);
      }
    }
    if (!all_sizes)
      all_sizes = xbt_dynar_new(sizeof(int), NULL);

    xbt_dynar_foreach(sizes, count, size) {
      /* Copy values of this one into all_sizes */
      int prevcount;
      if (xbt_dynar_length(all_sizes) <= count) {
        prevcount = size;
        xbt_dynar_set(all_sizes, count, &prevcount);
      } else {
        xbt_dynar_get_cpy(all_sizes, count, &prevcount);
        prevcount += size;
        xbt_dynar_set(all_sizes, count, &prevcount);
      }

      /* Report current sizes */
      if (count == 0)
        continue;
      if (size == 0)
        continue;
      printf("%uelm x %u cells; ", count, size);
    }
  }
  printf("\n");
  xbt_dynar_free(&sizes);
}

/**
 * Create the dict mallocators.
 * This is an internal XBT function called during the lib initialization.
 * It can be used several times to recreate the mallocator, for example when you switch to MC mode
 */
void xbt_dict_preinit(void)
{
  dict_elm_mallocator = xbt_mallocator_new(256,
                                           dict_elm_mallocator_new_f,
                                           dict_elm_mallocator_free_f,
                                           dict_elm_mallocator_reset_f);
  dict_het_elm_mallocator = xbt_mallocator_new(256,
                                               dict_het_elm_mallocator_new_f,
                                               dict_het_elm_mallocator_free_f,
                                               dict_het_elm_mallocator_reset_f);
}

/**
 * Destroy the dict mallocators.
 * This is an internal XBT function during the lib initialization
 */
void xbt_dict_postexit(void)
{
  if (dict_elm_mallocator != NULL) {
    xbt_mallocator_free(dict_elm_mallocator);
    dict_elm_mallocator = NULL;
    xbt_mallocator_free(dict_het_elm_mallocator);
    dict_het_elm_mallocator = NULL;
  }
  if (all_sizes) {
    unsigned int count;
    int size;
    double avg = 0;
    int total_count = 0;
    printf("Overall stats:");
    xbt_dynar_foreach(all_sizes, count, size) {
      if (count == 0)
        continue;
      if (size == 0)
        continue;
      printf("%uelm x %d cells; ", count, size);
      avg += count * size;
      total_count += size;
    }
    printf("; %f elm per cell\n", avg / (double) total_count);
  }
}

#ifdef SIMGRID_TEST
#include "xbt.h"
#include "xbt/ex.h"
#include "portable.h"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(xbt_dict);

XBT_TEST_SUITE("dict", "Dict data container");

static void debuged_add_ext(xbt_dict_t head, const char *key,
                            const char *data_to_fill, void_f_pvoid_t free_f)
{
  char *data = xbt_strdup(data_to_fill);

  xbt_test_log("Add %s under %s", PRINTF_STR(data_to_fill),
                PRINTF_STR(key));

  xbt_dict_set(head, key, data, free_f);
  if (XBT_LOG_ISENABLED(xbt_dict, xbt_log_priority_debug)) {
    xbt_dict_dump(head, (void (*)(void *)) &printf);
    fflush(stdout);
  }
}

static void debuged_add(xbt_dict_t head, const char *key, void_f_pvoid_t free_f)
{
  debuged_add_ext(head, key, key, free_f);
}

static void fill(xbt_dict_t * head, int homogeneous)
{
  void_f_pvoid_t free_f = homogeneous ? NULL : &free;

  xbt_test_add("Fill in the dictionnary");

  *head = homogeneous ? xbt_dict_new_homogeneous(&free) : xbt_dict_new();
  debuged_add(*head, "12", free_f);
  debuged_add(*head, "12a", free_f);
  debuged_add(*head, "12b", free_f);
  debuged_add(*head, "123", free_f);
  debuged_add(*head, "123456", free_f);
  /* Child becomes child of what to add */
  debuged_add(*head, "1234", free_f);
  /* Need of common ancestor */
  debuged_add(*head, "123457", free_f);
}


static void search_ext(xbt_dict_t head, const char *key, const char *data)
{
  char *found;

  xbt_test_add("Search %s", key);
  found = xbt_dict_get(head, key);
  xbt_test_log("Found %s", found);
  if (data) {
    xbt_test_assert(found,
                    "data do not match expectations: found NULL while searching for %s",
                    data);
    if (found)
      xbt_test_assert(!strcmp(data, found),
                      "data do not match expectations: found %s while searching for %s",
                      found, data);
  } else {
    xbt_test_assert(!found,
                    "data do not match expectations: found %s while searching for NULL",
                    found);
  }
}

static void search(xbt_dict_t head, const char *key)
{
  search_ext(head, key, key);
}

static void debuged_remove(xbt_dict_t head, const char *key)
{

  xbt_test_add("Remove '%s'", PRINTF_STR(key));
  xbt_dict_remove(head, key);
  /*  xbt_dict_dump(head,(void (*)(void*))&printf); */
}


static void traverse(xbt_dict_t head)
{
  xbt_dict_cursor_t cursor = NULL;
  char *key;
  char *data;
  int i = 0;

  xbt_dict_foreach(head, cursor, key, data) {
    if (!key || !data || strcmp(key, data)) {
      xbt_test_log("Seen #%d:  %s->%s", ++i, PRINTF_STR(key),
                    PRINTF_STR(data));
    } else {
      xbt_test_log("Seen #%d:  %s", ++i, PRINTF_STR(key));
    }
    xbt_test_assert(!data || !strcmp(key, data),
                     "Key(%s) != value(%s). Aborting", key, data);
  }
}

static void search_not_found(xbt_dict_t head, const char *data)
{
  int ok = 0;
  xbt_ex_t e;

  xbt_test_add("Search %s (expected not to be found)", data);

  TRY {
    data = xbt_dict_get(head, data);
    THROWF(unknown_error, 0,
           "Found something which shouldn't be there (%s)", data);
  }
  CATCH(e) {
    if (e.category != not_found_error)
      xbt_test_exception(e);
    xbt_ex_free(e);
    ok = 1;
  }
  xbt_test_assert(ok, "Exception not raised");
}

static void count(xbt_dict_t dict, int length)
{
  xbt_dict_cursor_t cursor;
  char *key;
  void *data;
  int effective = 0;


  xbt_test_add("Count elements (expecting %d)", length);
  xbt_test_assert(xbt_dict_length(dict) == length,
                   "Announced length(%d) != %d.", xbt_dict_length(dict),
                   length);

  xbt_dict_foreach(dict, cursor, key, data)
      effective++;

  xbt_test_assert(effective == length, "Effective length(%d) != %d.",
                   effective, length);

}

static void count_check_get_key(xbt_dict_t dict, int length)
{
  xbt_dict_cursor_t cursor;
  char *key;
  _XBT_GNUC_UNUSED char *key2;
  void *data;
  int effective = 0;


  xbt_test_add
      ("Count elements (expecting %d), and test the getkey function",
       length);
  xbt_test_assert(xbt_dict_length(dict) == length,
                   "Announced length(%d) != %d.", xbt_dict_length(dict),
                   length);

  xbt_dict_foreach(dict, cursor, key, data) {
    effective++;
    key2 = xbt_dict_get_key(dict, data);
    xbt_assert(!strcmp(key, key2),
                "The data was registered under %s instead of %s as expected",
                key2, key);
  }

  xbt_test_assert(effective == length, "Effective length(%d) != %d.",
                   effective, length);

}

xbt_ex_t e;
xbt_dict_t head = NULL;
char *data;

static void basic_test(int homogeneous)
{
  void_f_pvoid_t free_f;

  xbt_test_add("Traversal the null dictionary");
  traverse(head);

  xbt_test_add("Traversal and search the empty dictionary");
  head = homogeneous ? xbt_dict_new_homogeneous(&free) : xbt_dict_new();
  traverse(head);
  TRY {
    debuged_remove(head, "12346");
  }
  CATCH(e) {
    if (e.category != not_found_error)
      xbt_test_exception(e);
    xbt_ex_free(e);
  }
  xbt_dict_free(&head);

  free_f = homogeneous ? NULL : &free;

  xbt_test_add("Traverse the full dictionary");
  fill(&head, homogeneous);
  count_check_get_key(head, 7);

  debuged_add_ext(head, "toto", "tutu", free_f);
  search_ext(head, "toto", "tutu");
  debuged_remove(head, "toto");

  search(head, "12a");
  traverse(head);

  xbt_test_add("Free the dictionary (twice)");
  xbt_dict_free(&head);
  xbt_dict_free(&head);

  /* CHANGING */
  fill(&head, homogeneous);
  count_check_get_key(head, 7);
  xbt_test_add("Change 123 to 'Changed 123'");
  xbt_dict_set(head, "123", strdup("Changed 123"), free_f);
  count_check_get_key(head, 7);

  xbt_test_add("Change 123 back to '123'");
  xbt_dict_set(head, "123", strdup("123"), free_f);
  count_check_get_key(head, 7);

  xbt_test_add("Change 12a to 'Dummy 12a'");
  xbt_dict_set(head, "12a", strdup("Dummy 12a"), free_f);
  count_check_get_key(head, 7);

  xbt_test_add("Change 12a to '12a'");
  xbt_dict_set(head, "12a", strdup("12a"), free_f);
  count_check_get_key(head, 7);

  xbt_test_add("Traverse the resulting dictionary");
  traverse(head);

  /* RETRIEVE */
  xbt_test_add("Search 123");
  data = xbt_dict_get(head, "123");
  xbt_test_assert(data);
  xbt_test_assert(!strcmp("123", data));

  search_not_found(head, "Can't be found");
  search_not_found(head, "123 Can't be found");
  search_not_found(head, "12345678 NOT");

  search(head, "12a");
  search(head, "12b");
  search(head, "12");
  search(head, "123456");
  search(head, "1234");
  search(head, "123457");

  xbt_test_add("Traverse the resulting dictionary");
  traverse(head);

  /*  xbt_dict_dump(head,(void (*)(void*))&printf); */

  xbt_test_add("Free the dictionary twice");
  xbt_dict_free(&head);
  xbt_dict_free(&head);

  xbt_test_add("Traverse the resulting dictionary");
  traverse(head);
}

XBT_TEST_UNIT("basic_heterogeneous", test_dict_basic_heterogeneous, "Basic usage: change, retrieve, traverse: heterogeneous dict")
{
  basic_test(0);
}

XBT_TEST_UNIT("basic_homogeneous", test_dict_basic_homogeneous, "Basic usage: change, retrieve, traverse: homogeneous dict")
{
  basic_test(1);
}

static void remove_test(int homogeneous)
{
  fill(&head, homogeneous);
  count(head, 7);
  xbt_test_add("Remove non existing data");
  TRY {
    debuged_remove(head, "Does not exist");
  }
  CATCH(e) {
    if (e.category != not_found_error)
      xbt_test_exception(e);
    xbt_ex_free(e);
  }
  traverse(head);

  xbt_dict_free(&head);

  xbt_test_add
      ("Remove each data manually (traversing the resulting dictionary each time)");
  fill(&head, homogeneous);
  debuged_remove(head, "12a");
  traverse(head);
  count(head, 6);
  debuged_remove(head, "12b");
  traverse(head);
  count(head, 5);
  debuged_remove(head, "12");
  traverse(head);
  count(head, 4);
  debuged_remove(head, "123456");
  traverse(head);
  count(head, 3);
  TRY {
    debuged_remove(head, "12346");
  }
  CATCH(e) {
    if (e.category != not_found_error)
      xbt_test_exception(e);
    xbt_ex_free(e);
    traverse(head);
  }
  debuged_remove(head, "1234");
  traverse(head);
  debuged_remove(head, "123457");
  traverse(head);
  debuged_remove(head, "123");
  traverse(head);
  TRY {
    debuged_remove(head, "12346");
  }
  CATCH(e) {
    if (e.category != not_found_error)
      xbt_test_exception(e);
    xbt_ex_free(e);
  }
  traverse(head);

  xbt_test_add
      ("Free dict, create new fresh one, and then reset the dict");
  xbt_dict_free(&head);
  fill(&head, homogeneous);
  xbt_dict_reset(head);
  count(head, 0);
  traverse(head);

  xbt_test_add("Free the dictionary twice");
  xbt_dict_free(&head);
  xbt_dict_free(&head);
}

XBT_TEST_UNIT("remove_heterogeneous", test_dict_remove_heterogeneous, "Removing some values: heterogeneous dict")
{
  remove_test(0);
}

XBT_TEST_UNIT("remove_homogeneous", test_dict_remove_homogeneous, "Removing some values: homogeneous dict")
{
  remove_test(1);
}

XBT_TEST_UNIT("nulldata", test_dict_nulldata, "NULL data management")
{
  fill(&head, 1);

  xbt_test_add("Store NULL under 'null'");
  xbt_dict_set(head, "null", NULL, NULL);
  search_ext(head, "null", NULL);

  xbt_test_add("Check whether I see it while traversing...");
  {
    xbt_dict_cursor_t cursor = NULL;
    char *key;
    int found = 0;

    xbt_dict_foreach(head, cursor, key, data) {
      if (!key || !data || strcmp(key, data)) {
        xbt_test_log("Seen:  %s->%s", PRINTF_STR(key), PRINTF_STR(data));
      } else {
        xbt_test_log("Seen:  %s", PRINTF_STR(key));
      }

      if (!strcmp(key, "null"))
        found = 1;
    }
    xbt_test_assert(found,
                     "the key 'null', associated to NULL is not found");
  }
  xbt_dict_free(&head);
}

#define NB_ELM 20000
#define SIZEOFKEY 1024
static int countelems(xbt_dict_t head)
{
  xbt_dict_cursor_t cursor;
  char *key;
  void *data;
  int res = 0;

  xbt_dict_foreach(head, cursor, key, data) {
    res++;
  }
  return res;
}

XBT_TEST_UNIT("crash", test_dict_crash, "Crash test")
{
  xbt_dict_t head = NULL;
  int i, j, k;
  char *key;

  srand((unsigned int) time(NULL));

  for (i = 0; i < 10; i++) {
    xbt_test_add("CRASH test number %d (%d to go)", i + 1, 10 - i - 1);
    xbt_test_log
        ("Fill the struct, count its elems and frees the structure");
    xbt_test_log
        ("using 1000 elements with %d chars long randomized keys.",
         SIZEOFKEY);
    head = xbt_dict_new();
    /* if (i%10) printf("."); else printf("%d",i/10); fflush(stdout); */
    for (j = 0; j < 1000; j++) {
      char *data = NULL;
      key = xbt_malloc(SIZEOFKEY);

      do {
        for (k = 0; k < SIZEOFKEY - 1; k++)
          key[k] = rand() % ('z' - 'a') + 'a';
        key[k] = '\0';
        /*      printf("[%d %s]\n",j,key); */
        data = xbt_dict_get_or_null(head, key);
      } while (data != NULL);

      xbt_dict_set(head, key, key, &free);
      data = xbt_dict_get(head, key);
      xbt_test_assert(!strcmp(key, data),
                       "Retrieved value (%s) != Injected value (%s)", key,
                       data);

      count(head, j + 1);
    }
    /*    xbt_dict_dump(head,(void (*)(void*))&printf); */
    traverse(head);
    xbt_dict_free(&head);
    xbt_dict_free(&head);
  }


  head = xbt_dict_new();
  xbt_test_add("Fill %d elements, with keys being the number of element",
                NB_ELM);
  for (j = 0; j < NB_ELM; j++) {
    /* if (!(j%1000)) { printf("."); fflush(stdout); } */

    key = xbt_malloc(10);

    sprintf(key, "%d", j);
    xbt_dict_set(head, key, key, &free);
  }
  /*xbt_dict_dump(head,(void (*)(void*))&printf); */

  xbt_test_add
      ("Count the elements (retrieving the key and data for each)");
  i = countelems(head);
  xbt_test_log("There is %d elements", i);

  xbt_test_add("Search my %d elements 20 times", NB_ELM);
  key = xbt_malloc(10);
  for (i = 0; i < 20; i++) {
    void *data;
    /* if (i%10) printf("."); else printf("%d",i/10); fflush(stdout); */
    for (j = 0; j < NB_ELM; j++) {

      sprintf(key, "%d", j);
      data = xbt_dict_get(head, key);
      xbt_test_assert(!strcmp(key, (char *) data),
                       "with get, key=%s != data=%s", key, (char *) data);
      data = xbt_dict_get_ext(head, key, strlen(key));
      xbt_test_assert(!strcmp(key, (char *) data),
                       "with get_ext, key=%s != data=%s", key,
                       (char *) data);
    }
  }
  free(key);

  xbt_test_add("Remove my %d elements", NB_ELM);
  key = xbt_malloc(10);
  for (j = 0; j < NB_ELM; j++) {
    /* if (!(j%10000)) printf("."); fflush(stdout); */

    sprintf(key, "%d", j);
    xbt_dict_remove(head, key);
  }
  free(key);


  xbt_test_add("Free the structure (twice)");
  xbt_dict_free(&head);
  xbt_dict_free(&head);
}

#endif                          /* SIMGRID_TEST */
