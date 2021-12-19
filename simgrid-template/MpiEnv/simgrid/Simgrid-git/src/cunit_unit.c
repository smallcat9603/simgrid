/*******************************/
/* GENERATED FILE, DO NOT EDIT */
/*******************************/

#include <stdio.h>
#include "xbt.h"
/*******************************/
/* GENERATED FILE, DO NOT EDIT */
/*******************************/

#line 823 "xbt/cunit.c" 


XBT_TEST_UNIT("expect", test_expected_failure, "expected failures")
{
  xbt_test_add("Skipped test");
  xbt_test_skip();

  xbt_test_add("%s %s", "EXPECTED", "FAILURE");
  xbt_test_expect_failure();
  xbt_test_log("%s %s", "Test", "log");
  xbt_test_fail("EXPECTED FAILURE");
}

/*******************************/
/* GENERATED FILE, DO NOT EDIT */
/*******************************/

