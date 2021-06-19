/*  xbt/sysdep.h -- all system dependency                                   */
/*  no system header should be loaded out of this file so that we have only */
/*  one file to check when porting to another OS                            */

/* Copyright (c) 2004-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef _XBT_SYSDEP_H
#define _XBT_SYSDEP_H

#include "xbt/log.h"
#include "xbt/misc.h"
#include "xbt/asserts.h"

#include "simgrid_config.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>             /* va_list */

SG_BEGIN_DECL()

/* They live in asserts.h, but need to be declared before this module.
   double declaration to cut dependency cycle */
/**
 * @addtogroup XBT_error
 *
 * @{
 */
/** @brief Kill the program in silence */
XBT_PUBLIC(void) xbt_abort(void) _XBT_GNUC_NORETURN;

/**
 * @brief Kill the program with an error message
 * \param ... a format string and its arguments
 *
 * Things are so messed up that the only thing to do now, is to stop the
 * program.
 *
 * The message is handled by a CRITICAL logging request, and may consist of a
 * format string with arguments.
 */
#define xbt_die(...)                            \
  do {                                          \
    XBT_CCRITICAL(xbt, __VA_ARGS__);            \
    xbt_abort();                                \
  } while (0)
/** @} */

XBT_LOG_EXTERNAL_CATEGORY(xbt);

/* these ones live in str.h, but redeclare them here so that we do
   not need to load the whole str.h and its heavy dependencies */
#ifndef __USE_GNU               /* do not redeclare existing headers */
XBT_PUBLIC(int) asprintf(char **ptr, const char *fmt,   /*args */
                         ...) _XBT_GNUC_PRINTF(2, 3);
XBT_PUBLIC(int) vasprintf(char **ptr, const char *fmt, va_list ap);
#endif
XBT_PUBLIC(char *) bprintf(const char *fmt, ...) _XBT_GNUC_PRINTF(1, 2);

/** @addtogroup XBT_syscall
 *  @brief Malloc and associated functions, killing the program on error (with \ref XBT_ex)
 *
 *  @{
 */

#if defined(__GNUC__) || defined(DOXYGEN)
/** @brief Like strdup, but xbt_die() on error */
static inline __attribute__ ((always_inline))
char *xbt_strdup(const char *s)
{
  char *res = NULL;
  if (s) {
    res = strdup(s);
    if (!res)
      xbt_die("memory allocation error (strdup returned NULL)");
  }
  return res;
}

XBT_PUBLIC(void) xbt_backtrace_display_current(void);

/** @brief Like malloc, but xbt_die() on error
    @hideinitializer */
static inline __attribute__ ((always_inline))
void *xbt_malloc(size_t n)
{
  void *res;
/*  if (n==0) {
     xbt_backtrace_display_current();
     xbt_die("malloc(0) is not portable");
  }*/

  res = malloc(n);
  if (!res)
    xbt_die("Memory allocation of %lu bytes failed", (unsigned long)n);
  return res;
}

/** @brief like malloc, but xbt_die() on error and memset data to 0
    @hideinitializer */
static inline __attribute__ ((always_inline))
void *xbt_malloc0(size_t n)
{
  void *res;
  //if (n==0) xbt_die("calloc(0) is not portable");
  res = calloc(n, 1);
  if (!res)
    xbt_die("Memory callocation of %lu bytes failed", (unsigned long)n);
  return res;
}

/** @brief like realloc, but xbt_die() on error
    @hideinitializer */
static inline __attribute__ ((always_inline))
void *xbt_realloc(void *p, size_t s)
{
  void *res = NULL;
  //if (s==0) xbt_die("realloc(0) is not portable");
  if (s) {
    if (p) {
      res = realloc(p, s);
      if (!res)
        xbt_die("memory (re)allocation of %lu bytes failed", (unsigned long)s);
    } else {
      res = xbt_malloc(s);
    }
  } else {
    free(p);
  }
  return res;
}
#else                           /* non __GNUC__  */
#  define xbt_strdup(s)    strdup(s)
#  define xbt_malloc(n)    malloc(n)
#  define xbt_malloc0(n)   calloc(n,1)
#  define xbt_realloc(p,s) realloc(p,s)
#endif                          /* __GNUC__ ? */

/** @brief like free
    @hideinitializer */
#define xbt_free(p) free(p) /*nothing specific to do here. A poor valgrind replacement? */

/** @brief like free, but you can be sure that it is a function  */
XBT_PUBLIC(void) xbt_free_f(void *p);
/** @brief should be given a pointer to pointer, and frees the second one */
XBT_PUBLIC(void) xbt_free_ref(void *d);

/** @brief like calloc, but xbt_die() on error and don't memset to 0
    @hideinitializer */
#define xbt_new(type, count)  ((type*)xbt_malloc (sizeof (type) * (count)))
/** @brief like calloc, but xbt_die() on error
    @hideinitializer */
#define xbt_new0(type, count) ((type*)xbt_malloc0 (sizeof (type) * (count)))

/** @} */


SG_END_DECL()
#endif                          /* _XBT_SYSDEP_H */
