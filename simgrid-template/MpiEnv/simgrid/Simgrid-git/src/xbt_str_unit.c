/*******************************/
/* GENERATED FILE, DO NOT EDIT */
/*******************************/

#include <stdio.h>
#include "xbt.h"
/*******************************/
/* GENERATED FILE, DO NOT EDIT */
/*******************************/

#line 926 "xbt/xbt_str.c" 
#include "xbt/str.h"

#define mytest(name, input, expected) \
  xbt_test_add(name); \
  d=xbt_str_split_quoted(input); \
  s=xbt_str_join(d,"XXX"); \
  xbt_test_assert(!strcmp(s,expected),\
                   "Input (%s) leads to (%s) instead of (%s)", \
                   input,s,expected);\
                   free(s); \
                   xbt_dynar_free(&d);

XBT_TEST_UNIT("xbt_str_split_quoted", test_split_quoted, "test the function xbt_str_split_quoted")
{
  xbt_dynar_t d;
  char *s;

  mytest("Empty", "", "");
  mytest("Basic test", "toto tutu", "totoXXXtutu");
  mytest("Useless backslashes", "\\t\\o\\t\\o \\t\\u\\t\\u",
         "totoXXXtutu");
  mytest("Protected space", "toto\\ tutu", "toto tutu");
  mytest("Several spaces", "toto   tutu", "totoXXXtutu");
  mytest("LTriming", "  toto tatu", "totoXXXtatu");
  mytest("Triming", "  toto   tutu  ", "totoXXXtutu");
  mytest("Single quotes", "'toto tutu' tata", "toto tutuXXXtata");
  mytest("Double quotes", "\"toto tutu\" tata", "toto tutuXXXtata");
  mytest("Mixed quotes", "\"toto' 'tutu\" tata", "toto' 'tutuXXXtata");
  mytest("Backslashed quotes", "\\'toto tutu\\' tata",
         "'totoXXXtutu'XXXtata");
  mytest("Backslashed quotes + quotes", "'toto \\'tutu' tata",
         "toto 'tutuXXXtata");

}

#define mytest_str(name, input, separator, expected) \
  xbt_test_add(name); \
  d=xbt_str_split_str(input, separator); \
  s=xbt_str_join(d,"XXX"); \
  xbt_test_assert(!strcmp(s,expected),\
                   "Input (%s) leads to (%s) instead of (%s)", \
                   input,s,expected);\
                   free(s); \
                   xbt_dynar_free(&d);

XBT_TEST_UNIT("xbt_str_split_str", test_split_str, "test the function xbt_str_split_str")
{
  xbt_dynar_t d;
  char *s;

  mytest_str("Empty string and separator", "", "", "");
  mytest_str("Empty string", "", "##", "");
  mytest_str("Empty separator", "toto", "", "toto");
  mytest_str("String with no separator in it", "toto", "##", "toto");
  mytest_str("Basic test", "toto##tutu", "##", "totoXXXtutu");
}

/* Last args are format string and parameters for xbt_test_add */
#define mytest_diff(s1, s2, diff, ...)                                  \
  do {                                                                  \
    char *mytest_diff_res;                                              \
    xbt_test_add(__VA_ARGS__);                                          \
    mytest_diff_res = xbt_str_diff(s1, s2);                             \
    xbt_test_assert(!strcmp(mytest_diff_res, diff),                     \
                    "Wrong output:\n--- got:\n%s\n--- expected:\n%s\n---", \
                    mytest_diff_res, diff);                             \
    free(mytest_diff_res);                                              \
  } while (0)

XBT_TEST_UNIT("xbt_str_diff", test_diff, "test the function xbt_str_diff")
{
  unsigned i;

  /* Trivial cases */
  mytest_diff("a", "a", "  a", "1 word, no difference");
  mytest_diff("a", "A", "- a\n+ A", "1 word, different");
  mytest_diff("a\n", "a\n", "  a", "1 line, no difference");
  mytest_diff("a\n", "A\n", "- a\n+ A", "1 line, different");

  /* Empty strings */
  mytest_diff("", "", "", "empty strings");
  mytest_diff("", "a", "+ a", "1 word, added");
  mytest_diff("a", "", "- a", "1 word, removed");
  mytest_diff("", "a\n", "+ a", "1 line, added");
  mytest_diff("a\n", "", "- a", "1 line, removed");
  mytest_diff("", "a\nb\nc\n", "+ a\n+ b\n+ c", "4 lines, all added");
  mytest_diff("a\nb\nc\n", "", "- a\n- b\n- c", "4 lines, all removed");

  /* Empty lines */
  mytest_diff("\n", "\n", "  ", "empty lines");
  mytest_diff("", "\n", "+ ", "empty line, added");
  mytest_diff("\n", "", "- ", "empty line, removed");

  mytest_diff("a", "\na", "+ \n  a", "empty line added before word");
  mytest_diff("a", "a\n\n", "  a\n+ ", "empty line added after word");
  mytest_diff("\na", "a", "- \n  a", "empty line removed before word");
  mytest_diff("a\n\n", "a", "  a\n- ", "empty line removed after word");

  mytest_diff("a\n", "\na\n", "+ \n  a", "empty line added before line");
  mytest_diff("a\n", "a\n\n", "  a\n+ ", "empty line added after line");
  mytest_diff("\na\n", "a\n", "- \n  a", "empty line removed before line");
  mytest_diff("a\n\n", "a\n", "  a\n- ", "empty line removed after line");

  mytest_diff("a\nb\nc\nd\n", "\na\nb\nc\nd\n", "+ \n  a\n  b\n  c\n  d",
              "empty line added before 4 lines");
  mytest_diff("a\nb\nc\nd\n", "a\nb\nc\nd\n\n", "  a\n  b\n  c\n  d\n+ ",
              "empty line added after 4 lines");
  mytest_diff("\na\nb\nc\nd\n", "a\nb\nc\nd\n", "- \n  a\n  b\n  c\n  d",
              "empty line removed before 4 lines");
  mytest_diff("a\nb\nc\nd\n\n", "a\nb\nc\nd\n", "  a\n  b\n  c\n  d\n- ",
              "empty line removed after 4 lines");

  /* Missing newline at the end of one of the strings */
  mytest_diff("a\n", "a", "  a", "1 line, 1 word, no difference");
  mytest_diff("a", "a\n", "  a", "1 word, 1 line, no difference");
  mytest_diff("a\n", "A", "- a\n+ A", "1 line, 1 word, different");
  mytest_diff("a", "A\n", "- a\n+ A", "1 word, 1 line, different");

  mytest_diff("a\nb\nc\nd", "a\nb\nc\nd\n", "  a\n  b\n  c\n  d",
              "4 lines, no newline on first");
  mytest_diff("a\nb\nc\nd\n", "a\nb\nc\nd", "  a\n  b\n  c\n  d",
              "4 lines, no newline on second");

  /* Four lines, all combinations of differences */
  for (i = 0 ; i < (1U << 4) ; i++) {
    char d2[4 + 1];
    char s2[4 * 2 + 1];
    char res[4 * 8 + 1];
    char *pd = d2;
    char *ps = s2;
    char *pr = res;
    unsigned j = 0;
    while (j < 4) {
      unsigned k;
      for (/* j */ ; j < 4 && !(i & (1U << j)) ; j++) {
        *pd++ = "abcd"[j];
        ps += sprintf(ps, "%c\n", "abcd"[j]);
        pr += sprintf(pr, "  %c\n", "abcd"[j]);
      }
      for (k = j ; k < 4 && (i & (1U << k)) ; k++) {
        *pd++ = "ABCD"[k];
        ps += sprintf(ps, "%c\n", "ABCD"[k]);
        pr += sprintf(pr, "- %c\n", "abcd"[k]);
      }
      for (/* j */ ; j < k ; j++) {
        pr += sprintf(pr, "+ %c\n", "ABCD"[j]);
      }
    }
    *pd = '\0';
    *--pr = '\0';               /* strip last '\n' from expected result */
    mytest_diff("a\nb\nc\nd\n", s2, res,
                "compare (abcd) with changed (%s)", d2);
  }

  /* Subsets of four lines, do not test for empty subset */
  for (i = 1 ; i < (1U << 4) ; i++) {
    char d2[4 + 1];
    char s2[4 * 2 + 1];
    char res[4 * 8 + 1];
    char *pd = d2;
    char *ps = s2;
    char *pr = res;
    unsigned j = 0;
    while (j < 4) {
      for (/* j */ ; j < 4 && (i & (1U << j)) ; j++) {
        *pd++ = "abcd"[j];
        ps += sprintf(ps, "%c\n", "abcd"[j]);
        pr += sprintf(pr, "  %c\n", "abcd"[j]);
      }
      for (/* j */; j < 4 && !(i & (1U << j)) ; j++) {
        pr += sprintf(pr, "- %c\n", "abcd"[j]);
      }
    }
    *pd = '\0';
    *--pr = '\0';               /* strip last '\n' from expected result */
    mytest_diff("a\nb\nc\nd\n", s2, res,
                "compare (abcd) with subset (%s)", d2);

    for (pr = res ; *pr != '\0' ; pr++)
      if (*pr == '-')
        *pr = '+';
    mytest_diff(s2, "a\nb\nc\nd\n", res,
                "compare subset (%s) with (abcd)", d2);
  }
}

/*******************************/
/* GENERATED FILE, DO NOT EDIT */
/*******************************/

