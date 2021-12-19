/* simgrid_config.h - Results of the configure made visible to user code    */

/* Copyright (c) 2009-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt/misc.h"           /* XBT_PUBLIC */
#include "xbt/dynar.h"          /* sg_commandline */

#ifndef SIMGRID_PUBLIC_CONFIG_H
#define SIMGRID_PUBLIC_CONFIG_H
SG_BEGIN_DECL()


/** Define the version numbers of the used header files. 
    sg_version() can be used to retrieve the version of the dynamic library.
    But actually, if these numbers don't match, SimGrid refuses to start (so you shouldn't have to care about sg_version() yourself) */

#define SIMGRID_VERSION_MAJOR 3
#define SIMGRID_VERSION_MINOR 12
#define SIMGRID_VERSION_PATCH 0

#define SIMGRID_VERSION_STRING "SimGrid version 3.12-devel\nCopyright (c) 2004-2014. The Simgrid Team.\nRelease build at commit c87fbc1 (2014-11-24 15:33:47 +0100)"

/* Version X.Y.Z will get version number XYZ: all digits concatenated without dots
 * (with Y and Z must be on two positions)*/

#define MAKE_SIMGRID_VERSION(major, minor, patch)       \
  (100UL * (100UL * (major) + (minor)) + (patch))
#define SIMGRID_VERSION MAKE_SIMGRID_VERSION(SIMGRID_VERSION_MAJOR, \
                                             SIMGRID_VERSION_MINOR, \
                                             SIMGRID_VERSION_PATCH)

/** Retrieves the version numbers of the used dynamic library (so, DLL or dynlib) , while
    SIMGRID_VERSION_MAJOR and friends give the version numbers of the used header files */
XBT_PUBLIC(void) sg_version(int *major,int *minor,int *patch);

/** Contains all the parameters we got from the command line */
XBT_PUBLIC_DATA(xbt_dynar_t) sg_cmdline;

/* take care of DLL usage madness */

#ifdef _XBT_DLL_EXPORT
	#ifndef DLL_EXPORT
		#define DLL_EXPORT
	#endif
#else
	#ifdef _XBT_DLL_STATIC
		#ifndef DLL_STATIC
			#define DLL_STATIC
		#endif
	#else
		#ifndef DLL_EXPORT
			#define DLL_IMPORT
		#endif
	#endif
#endif

/* #undef _XBT_WIN32 */
/* #undef _WIN32 */
/* #undef _WIN64 */
/* #undef __VISUALC__ */
/* #undef __BORLANDC__ */
#ifdef _XBT_WIN32
	#ifndef __GNUC__
/* #undef __GNUC__ */
	#endif
#endif


/* Define to 1 if mmalloc is compiled in. */
/* #undef HAVE_MMALLOC */

/* Get the config */
#undef SIMGRID_NEED_ASPRINTF
#undef SIMGRID_NEED_VASPRINTF



#include <stdarg.h>

/* snprintf related functions */
/** @addtogroup XBT_str
  * @{ */
/** @brief print to allocated string (reimplemented when not provided by the system)
 *
 * The functions asprintf() and vasprintf() are analogues of
 * sprintf() and vsprintf(), except that they allocate a string large
 * enough to hold the output including the terminating null byte, and
 * return a pointer to it via the first parameter.  This pointer
 * should be passed to free(3) to release the allocated storage when
 * it is no longer needed.
 */
#if defined(SIMGRID_NEED_ASPRINTF)||defined(DOXYGEN)
XBT_PUBLIC(int) asprintf(char **ptr, const char *fmt,   /*args */
                         ...) _XBT_GNUC_PRINTF(2, 3);
#endif
/** @brief print to allocated string (reimplemented when not provided by the system)
 *
 * See asprintf()
 */
#if defined(SIMGRID_NEED_VASPRINTF)||defined(DOXYGEN)
XBT_PUBLIC(int) vasprintf(char **ptr, const char *fmt, va_list ap);
#endif
/** @brief print to allocated string
 *
 * Works just like vasprintf(), but returns a pointer to the newly
 * created string, or aborts on error.
 */
XBT_PUBLIC(char *) bvprintf(const char *fmt, va_list ap);
/** @brief print to allocated string
 *
 * Works just like asprintf(), but returns a pointer to the newly
 * created string, or aborts on error.
 */
XBT_PUBLIC(char *) bprintf(const char *fmt, ...) _XBT_GNUC_PRINTF(1, 2);
/** @} */

/* Whether mallocators were enabled in ccmake or not. */
#define MALLOCATOR_COMPILED_IN 1

/* Define if xbt contexts are based on our threads implementation or not */
#define CONTEXT_THREADS 1

/* Tracing SimGrid */
#define HAVE_TRACING 1

/* Jedule output  */
/* #undef HAVE_JEDULE */

/* Tracking of latency bound */
/* #undef HAVE_LATENCY_BOUND_TRACKING */

/* If __thread is available */
#define HAVE_THREAD_LOCAL_STORAGE 1

/* If Model-Checking support was requested */
/* #undef HAVE_MC */

SG_END_DECL()
#endif /* SIMGRID_PUBLIC_CONFIG_H */
