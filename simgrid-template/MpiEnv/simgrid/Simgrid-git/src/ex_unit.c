/*******************************/
/* GENERATED FILE, DO NOT EDIT */
/*******************************/

#include <stdio.h>
#include "xbt.h"
/*******************************/
/* GENERATED FILE, DO NOT EDIT */
/*******************************/

#line 239 "xbt/ex.c" 
#include <stdio.h>
#include "xbt/ex.h"


XBT_TEST_UNIT("controlflow", test_controlflow, "basic nested control flow")
{
  xbt_ex_t ex;
  volatile int n = 1;

  xbt_test_add("basic nested control flow");

  TRY {
    if (n != 1)
      xbt_test_fail("M1: n=%d (!= 1)", n);
    n++;
    TRY {
      if (n != 2)
        xbt_test_fail("M2: n=%d (!= 2)", n);
      n++;
      THROWF(unknown_error, 0, "something");
    }
    CATCH(ex) {
      if (n != 3)
        xbt_test_fail("M3: n=%d (!= 3)", n);
      n++;
      xbt_ex_free(ex);
    }
    n++;
    TRY {
      if (n != 5)
        xbt_test_fail("M2: n=%d (!= 5)", n);
      n++;
      THROWF(unknown_error, 0, "something");
    }
    CATCH_ANONYMOUS {
      if (n != 6)
        xbt_test_fail("M3: n=%d (!= 6)", n);
      n++;
      RETHROW;
      n++;
    }
    xbt_test_fail("MX: n=%d (shouldn't reach this point)", n);
  }
  CATCH(ex) {
    if (n != 7)
      xbt_test_fail("M4: n=%d (!= 7)", n);
    n++;
    xbt_ex_free(ex);
  }
  if (n != 8)
    xbt_test_fail("M5: n=%d (!= 8)", n);
}

XBT_TEST_UNIT("value", test_value, "exception value passing")
{
  xbt_ex_t ex;

  TRY {
    THROWF(unknown_error, 2, "toto");
  }
  CATCH(ex) {
    xbt_test_add("exception value passing");
    if (ex.category != unknown_error)
      xbt_test_fail("category=%d (!= 1)", (int)ex.category);
    if (ex.value != 2)
      xbt_test_fail("value=%d (!= 2)", ex.value);
    if (strcmp(ex.msg, "toto"))
      xbt_test_fail("message=%s (!= toto)", ex.msg);
    xbt_ex_free(ex);
  }
}

XBT_TEST_UNIT("variables", test_variables, "variable value preservation")
{
  xbt_ex_t ex;
  int r1;
  int _XBT_GNUC_UNUSED r2;
  int v1;
  volatile int v2;

  r1 = r2 = v1 = v2 = 1234;
  TRY {
    r2 = 5678;
    v2 = 5678;
    THROWF(unknown_error, 0, "toto");
  }
  CATCH(ex) {
    xbt_test_add("variable preservation");
    if (r1 != 1234)
      xbt_test_fail("r1=%d (!= 1234)", r1);
    if (v1 != 1234)
      xbt_test_fail("v1=%d (!= 1234)", v1);
    /* r2 is allowed to be destroyed because not volatile */
    if (v2 != 5678)
      xbt_test_fail("v2=%d (!= 5678)", v2);
    xbt_ex_free(ex);
  }
}

XBT_TEST_UNIT("cleanup", test_cleanup, "cleanup handling")
{
  xbt_ex_t ex;
  volatile int v1;
  int c;

  xbt_test_add("cleanup handling");

  v1 = 1234;
  c = 0;
  TRY {
    v1 = 5678;
    THROWF(1, 2, "blah");
  }
  TRY_CLEANUP {
    if (v1 != 5678)
      xbt_test_fail("v1 = %d (!= 5678)", v1);
    c = 1;
  }
  CATCH(ex) {
    if (v1 != 5678)
      xbt_test_fail("v1 = %d (!= 5678)", v1);
    if (!(ex.category == 1 && ex.value == 2 && !strcmp(ex.msg, "blah")))
      xbt_test_fail("unexpected exception contents");
    xbt_ex_free(ex);
  }
  if (!c)
    xbt_test_fail("xbt_ex_free not executed");
}


/*
 * The following is the example included in the documentation. It's a good
 * idea to check its syntax even if we don't try to run it.
 * And actually, it allows to put comments in the code despite doxygen.
 */
static char *mallocex(int size)
{
  return NULL;
}

#define SMALLAMOUNT 10
#define TOOBIG 100000000

#if 0                           /* this contains syntax errors, actually */
static void bad_example(void)
{
  struct {
    char *first;
  } *globalcontext;
  ex_t ex;

  /* BAD_EXAMPLE */
  TRY {
    char *cp1, *cp2, *cp3;

    cp1 = mallocex(SMALLAMOUNT);
    globalcontext->first = cp1;
    cp2 = mallocex(TOOBIG);
    cp3 = mallocex(SMALLAMOUNT);
    strcpy(cp1, "foo");
    strcpy(cp2, "bar");
  }
  TRY_CLEANUP {
    free(cp3);
    free(cp2);
    free(cp1);
  }
  CATCH_ANONYMOUS {
    printf("cp3=%s", cp3);
    RETHROW;
  }
  /* end_of_bad_example */
}
#endif
typedef struct {
  char *first;
} global_context_t;

static void good_example(void)
{
  global_context_t *global_context = xbt_malloc(sizeof(global_context_t));

  /* GOOD_EXAMPLE */
  {                             /*01 */
    char *volatile /*03 */ cp1 = NULL /*02 */ ;
    char *volatile /*03 */ cp2 = NULL /*02 */ ;
    char *volatile /*03 */ cp3 = NULL /*02 */ ;
    TRY {
      cp1 = mallocex(SMALLAMOUNT);
      global_context->first = cp1;
      cp1 = NULL /*05 give away */ ;
      cp2 = mallocex(TOOBIG);
      cp3 = mallocex(SMALLAMOUNT);
      strcpy(cp1, "foo");
      strcpy(cp2, "bar");
    }
    TRY_CLEANUP {               /*04 */
      printf("cp3=%s", cp3 == NULL /*02 */ ? "" : cp3);
      free(cp3);
      free(cp2);
      /*05 cp1 was given away */
    }
    CATCH_ANONYMOUS {
      /*05 global context untouched */
      RETHROW;
    }
  }
  /* end_of_good_example */
}
/*******************************/
/* GENERATED FILE, DO NOT EDIT */
/*******************************/

