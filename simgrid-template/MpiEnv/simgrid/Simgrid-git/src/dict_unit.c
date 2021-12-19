/*******************************/
/* GENERATED FILE, DO NOT EDIT */
/*******************************/

#include <stdio.h>
#include "xbt.h"
/*******************************/
/* GENERATED FILE, DO NOT EDIT */
/*******************************/

#line 678 "xbt/dict.c" 
#include "xbt.h"
#include "xbt/ex.h"
#include "portable.h"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(xbt_dict);


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

/*******************************/
/* GENERATED FILE, DO NOT EDIT */
/*******************************/

