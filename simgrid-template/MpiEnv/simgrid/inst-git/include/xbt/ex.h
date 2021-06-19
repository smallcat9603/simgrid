/* ex - Exception Handling                                                  */

/* Copyright (c) 2005-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/*  Copyright (c) 2002-2004 Ralf S. Engelschall <rse@engelschall.com>       */
/*  Copyright (c) 2002-2004 The OSSP Project <http://www.ossp.org/>         */
/*  Copyright (c) 2002-2004 Cable & Wireless <http://www.cw.com/>           */
/*  All rights reserved.                                                    */

/* This code is inspirated from the OSSP version (as retrieved back in 2004)*/
/* It was heavily modified to fit the SimGrid framework.                    */

/* The OSSP version has the following copyright notice:
**  OSSP ex - Exception Handling
**  Copyright (c) 2002-2004 Ralf S. Engelschall <rse@engelschall.com>
**  Copyright (c) 2002-2004 The OSSP Project <http://www.ossp.org/>
**  Copyright (c) 2002-2004 Cable & Wireless <http://www.cw.com/>
**
**  This file is part of OSSP ex, an exception handling library
**  which can be found at http://www.ossp.org/pkg/lib/ex/.
**
**  Permission to use, copy, modify, and distribute this software for
**  any purpose with or without fee is hereby granted, provided that
**  the above copyright notice and this permission notice appear in all
**  copies.
**
**  THIS SOFTWARE IS PROVIDED `AS IS'' AND ANY EXPRESSED OR IMPLIED
**  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
**  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
**  IN NO EVENT SHALL THE AUTHORS AND COPYRIGHT HOLDERS AND THEIR
**  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
**  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
**  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
**  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
**  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
**  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
**  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
**  SUCH DAMAGE.
 */

/* The extensions made for the SimGrid project can either be distributed    */
/* under the same license, or under the LGPL v2.1                           */

#ifndef __XBT_EX_H__
#define __XBT_EX_H__

#include "xbt/sysdep.h"
#include "xbt/misc.h"
#include "xbt/virtu.h"

SG_BEGIN_DECL()

/*-*-* Emergency debuging: define this when the exceptions get crazy *-*-*/
#undef __EX_MAYDAY
#ifdef __EX_MAYDAY
# include <stdio.h>
#include <errno.h>
#  define MAYDAY_SAVE(m)    printf("%d %s:%d save %p\n",                \
                                   xbt_getpid(), __FILE__, __LINE__,    \
                                   (m)->jb                              \
                                  ),
#  define MAYDAY_RESTORE(m) printf("%d %s:%d restore %p\n",             \
                                   xbt_getpid(), __FILE__, __LINE__,    \
                                   (m)->jb                              \
                                  ),
#  define MAYDAY_CATCH(e)   printf("%d %s:%d Catched '%s'\n",           \
                                   xbt_getpid(), __FILE__, __LINE__,    \
                                   (e).msg                              \
          ),
#else
#  define MAYDAY_SAVE(m)
#  define MAYDAY_RESTORE(m)
#  define MAYDAY_CATCH(e)
#endif
/*-*-* end of debugging stuff *-*-*/
#if defined(__EX_MCTX_MCSC__)
#include <ucontext.h>           /* POSIX.1 ucontext(3) */
#define __ex_mctx_struct         ucontext_t uc;
#define __ex_mctx_save(mctx)     (getcontext(&(mctx)->uc) == 0)
#define __ex_mctx_restored(mctx)        /* noop */
#define __ex_mctx_restore(mctx)  (void)setcontext(&(mctx)->uc)
#elif defined(__EX_MCTX_SSJLJ__)
#include <setjmp.h>             /* POSIX.1 sigjmp_buf(3) */
#define __ex_mctx_struct         sigjmp_buf jb;
#define __ex_mctx_save(mctx)     (sigsetjmp((mctx)->jb, 1) == 0)
#define __ex_mctx_restored(mctx)        /* noop */
#define __ex_mctx_restore(mctx)  (void)siglongjmp((mctx)->jb, 1)
#elif defined(__EX_MCTX_SJLJ__) || !defined(__EX_MCTX_CUSTOM__) || defined(__EX_MAYDAY)
#include <setjmp.h>             /* ISO-C jmp_buf(3) */
#define __ex_mctx_struct         jmp_buf jb;
#define __ex_mctx_save(mctx)     ( MAYDAY_SAVE(mctx) setjmp((mctx)->jb) == 0)
#define __ex_mctx_restored(mctx)        /* noop */
#define __ex_mctx_restore(mctx)  ( MAYDAY_RESTORE(mctx) (void)longjmp((mctx)->jb, 1))
#endif
/* declare the machine context type */
typedef struct {
__ex_mctx_struct} __ex_mctx_t;

/** @addtogroup XBT_ex
 *  @brief A set of macros providing exception a la C++ in ANSI C (grounding feature)
 *
 * This module is a small ISO-C++ style exception handling library
 * for use in the ISO-C language. It allows you to use the paradigm 
 * of throwing and catching exceptions in order to reduce the amount
 * of error handling code without hindering program robustness.
 *               
 * This is achieved by directly transferring exceptional return codes
 * (and the program control flow) from the location where the exception
 * is raised (throw point) to the location where it is handled (catch
 * point) -- usually from a deeply nested sub-routine to a parent 
 * routine. All intermediate routines no longer have to make sure that 
 * the exceptional return codes from sub-routines are correctly passed 
 * back to the parent.
 *
 * These features are brought to you by a modified version of the libex 
 * library, one of the numerous masterpiece of Ralf S. Engelschall.
 *
 * \htmlonly <div class="toc">\endhtmlonly
 *
 * @section XBT_ex_toc TABLE OF CONTENTS
 *
 *  - \ref XBT_ex_intro
 *  - \ref XBT_ex_base
 *  - \ref XBT_ex_pitfalls
 *
 * \htmlonly </div> \endhtmlonly
 *
 * @section XBT_ex_intro DESCRIPTION
 * 
 * In SimGrid, an exception is a triple <\a msg , \a category , \a value> 
 * where \a msg is a human-readable text describing the exceptional 
 * condition, \a code an integer describing what went wrong and \a value
 * providing a sort of sub-category. (this is different in the original libex).
 *
 * @section XBT_ex_base BASIC USAGE
 *
 * \em TRY \b TRIED_BLOCK [\em TRY_CLEANUP \b CLEANUP_BLOCK] \em CATCH (variable) \b CATCH_BLOCK
 *
 * This is the primary syntactical construct provided. It is modeled after the
 * ISO-C++ try-catch clause and should sound familiar to most of you.
 *
 * Any exception thrown directly from the TRIED_BLOCK block or from called
 * subroutines is caught. Cleanups which must be done after this block
 * (whenever an exception arose or not) should be placed into the optionnal
 * CLEANUP_BLOCK. The code dealing with the exceptions when they arise should
 * be placed into the (mandatory) CATCH_BLOCK.
 *
 * 
 * In absence of exception, the control flow goes into the blocks TRIED_BLOCK
 * and CLEANUP_BLOCK (if present); The CATCH_BLOCK block is then ignored.
 *
 * When an exception is thrown, the control flow goes through the following
 * blocks: TRIED_BLOCK (up to the statement throwing the exception),
 * CLEANUP_BLOCK (if any) and CATCH_BLOCK. The exception is stored in a
 * variable for inspection inside the CATCH_BLOCK. This variable must be
 * declared in the outter scope, but its value is only valid within the
 * CATCH_BLOCK block. 
 *
 * Some notes:
 *  - TRY, CLEANUP and CATCH cannot be used separately, they work
 *    only in combination and form a language clause as a whole.
 *  - In contrast to the syntax of other languages (such as C++ or Jave) there
 *    is only one CATCH block and not multiple ones (all exceptions are
 *    of the same \em xbt_ex_t C type). 
 *  - the variable of CATCH can naturally be reused in subsequent 
 *    CATCH clauses.
 *  - it is possible to nest TRY clauses.
 *
 * The TRY block is a regular ISO-C language statement block, but
 * 
 * <center><b>it is not
 * allowed to jump into it via "goto" or longjmp(3) or out of it via "break",
 * "return", "goto" or longjmp(3)</b>.</center>
 *
 * This is because there is some hidden setup and
 * cleanup that needs to be done regardless of whether an exception is
 * caught. Bypassing these steps will break the exception handling facility.
 * The symptom are likely to be a segfault at the next exception raising point,  
 * ie far away from the point where you did the mistake. If you suspect
 * that kind of error in your code, have a look at the little script
 * <tt>tools/xbt_exception_checker</tt> in the CVS. It extracts all the TRY
 * blocks from a set of C files you give it and display them (and only
 * them) on the standard output. You can then grep for the forbidden 
 * keywords on that output.
 *   
 * The CLEANUP and CATCH blocks are regular ISO-C language statement
 * blocks without any restrictions. You are even allowed to throw (and, in the
 * CATCH block, to re-throw) exceptions.
 *
 * There is one subtle detail you should remember about TRY blocks:
 * Variables used in the CLEANUP or CATCH clauses must be declared with
 * the storage class "volatile", otherwise they might contain outdated
 * information if an exception is thrown.
 *
 *
 * This is because you usually do not know which commands in the TRY
 * were already successful before the exception was thrown (logically speaking)
 * and because the underlying ISO-C setjmp(3) facility applies those
 * restrictions (technically speaking). As a matter of fact, value changes
 * between the TRY and the THROW may be discarded if you forget the
 * "volatile" keyword. 
 * 
 * \section XBT_ex_pitfalls PROGRAMMING PITFALLS 
 *
 * Exception handling is a very elegant and efficient way of dealing with
 * exceptional situation. Nevertheless it requires additional discipline in
 * programming and there are a few pitfalls one must be aware of. Look the
 * following code which shows some pitfalls and contains many errors (assuming
 * a mallocex() function which throws an exception if malloc(3) fails):
 *
 * \dontinclude ex.c
 * \skip BAD_EXAMPLE
 * \until end_of_bad_example
 *
 * This example raises a few issues:
 *  -# \b variable \b scope \n
 *     Variables which are used in the CLEANUP or CATCH clauses must be
 *     declared before the TRY clause, otherwise they only exist inside the
 *     TRY block. In the example above, cp1, cp2 and cp3 only exist in the
 *     TRY block and are invisible from the CLEANUP and CATCH
 *     blocks.
 *  -# \b variable \b initialization \n
 *     Variables which are used in the CLEANUP or CATCH clauses must
 *     be initialized before the point of the first possible THROW is
 *     reached. In the example above, CLEANUP would have trouble using cp3
 *     if mallocex() throws a exception when allocating a TOOBIG buffer.
 *  -# \b volatile \b variable \n
 *     Variables which are used in the CLEANUP or CATCH clauses MUST BE
 *     DECLARED AS "volatile", otherwise they might contain outdated
 *     information when an exception is thrown. 
 *  -# \b clean \b before \b catch \n
 *     The CLEANUP clause is not only place before the CATCH clause in
 *     the source code, it also occures before in the control flow. So,
 *     resources being cleaned up cannot be used in the CATCH block. In the
 *     example, c3 gets freed before the printf placed in CATCH.
 *  -# \b variable \b uninitialization \n
 *     If resources are passed out of the scope of the
 *     TRY/CLEANUP/CATCH construct, they naturally shouldn't get
 *     cleaned up. The example above does free(3) cp1 in CLEANUP although
 *     its value was affected to globalcontext->first, invalidating this
 *     pointer.

 * The following is fixed version of the code (annotated with the pitfall items
 * for reference): 
 *
 * \skip GOOD_EXAMPLE
 * \until end_of_good_example
 *
 * @{
 */

/** @brief different kind of errors */
typedef enum {
  unknown_error = 0,            /**< unknown error */
  arg_error,                    /**< Invalid argument */
  bound_error,                  /**< Out of bounds argument */
  mismatch_error,               /**< The provided ID does not match */
  not_found_error,              /**< The searched element was not found */
  system_error,                 /**< a syscall did fail */
  network_error,                /**< error while sending/receiving data */
  timeout_error,                /**< not quick enough, dude */
  cancel_error,                 /**< an action was canceled */
  thread_error,                 /**< error while [un]locking */
  host_error,                   /**< host failed */
  tracing_error,                /**< error during the simulation tracing */
  io_error,                     /**< disk or file error */
  vm_error                      /**< vm  error */
} xbt_errcat_t;

XBT_PUBLIC(const char *) xbt_ex_catname(xbt_errcat_t cat);

/** @brief Structure describing an exception */
typedef struct {
  char *msg;                    /**< human readable message */
  xbt_errcat_t category;        /**< category like HTTP (what went wrong) */
  int value;                    /**< like errno (why did it went wrong) */
  /* throw point */
  char *procname;               /**< Name of the process who thrown this */
  int pid;                      /**< PID of the process who thrown this */
  char *file;                   /**< Thrown point */
  int line;                     /**< Thrown point */
  char *func;                   /**< Thrown point */
  /* Backtrace */
  int used;
  char **bt_strings;            /* only filed on display (or before the network propagation) */
  void *bt[XBT_BACKTRACE_SIZE];
} xbt_ex_t;

/* declare the running context type
 * (that's where we get the process name for the logs and the exception storage)
 *  -- do not mess with it --
 */
typedef struct {
  __ex_mctx_t *ctx_mctx;        /* permanent machine context of enclosing try/catch */
  int ctx_caught;               /* temporary flag whether exception was caught */
  volatile xbt_ex_t exception;  /* temporary exception storage */
} xbt_running_ctx_t;

/* the static and dynamic initializers for a context structure */
#define XBT_RUNNING_CTX_INITIALIZER                             \
    { NULL, 0, { /* content */ NULL, unknown_error, 0,          \
                 /* throw point*/ NULL, 0, NULL, 0, NULL,       \
                 /* backtrace */ 0, NULL, /* bt[] */ } }

XBT_PUBLIC_DATA(const xbt_running_ctx_t) __xbt_ex_ctx_initializer;
#define XBT_RUNNING_CTX_INITIALIZE(ctx) (*(ctx) = __xbt_ex_ctx_initializer)

/* the exception context */
typedef xbt_running_ctx_t *(*xbt_running_ctx_fetcher_t) (void);
XBT_PUBLIC_DATA(xbt_running_ctx_fetcher_t) __xbt_running_ctx_fetch;
XBT_PUBLIC( xbt_running_ctx_t *)__xbt_ex_ctx_default(void);

/* the termination handler */
typedef void (*ex_term_cb_t) (xbt_ex_t *);
XBT_PUBLIC_DATA(ex_term_cb_t) __xbt_ex_terminate;
XBT_PUBLIC( void )__xbt_ex_terminate_default(xbt_ex_t * e);

/** @brief Introduce a block where exception may be dealed with 
 *  @hideinitializer
 */
#define TRY \
    { \
        xbt_running_ctx_t *__xbt_ex_ctx_ptr = __xbt_running_ctx_fetch(); \
        int __ex_cleanup = 0; \
        __ex_mctx_t *__ex_mctx_en; \
        __ex_mctx_t __ex_mctx_me; \
        __ex_mctx_en = __xbt_ex_ctx_ptr->ctx_mctx; \
        __xbt_ex_ctx_ptr->ctx_mctx = &__ex_mctx_me; \
        if (__ex_mctx_save(&__ex_mctx_me)) { \
            if (1)

/** @brief optional(!) block for cleanup
 *  @hideinitializer
 */
#define TRY_CLEANUP \
            else { \
            } \
            __xbt_ex_ctx_ptr->ctx_caught = 0; \
        } else { \
            __ex_mctx_restored(&__ex_mctx_me); \
            __xbt_ex_ctx_ptr->ctx_caught = 1; \
        } \
        __xbt_ex_ctx_ptr->ctx_mctx = __ex_mctx_en; \
        __ex_cleanup = 1; \
        if (1) { \
            if (1)

#ifndef DOXYGEN_SKIP
#  ifdef __cplusplus
#    define XBT_EX_T_CPLUSPLUSCAST (xbt_ex_t&)
#  else
#    define XBT_EX_T_CPLUSPLUSCAST
#  endif
#endif

/** @brief the block for catching (ie, deal with) an exception 
 *  @hideinitializer
 */
#define CATCH(e) \
  DO_CATCH((e) = XBT_EX_T_CPLUSPLUSCAST __xbt_running_ctx_fetch()->exception)

/** @brief like CATCH(e) but without argument
 *  @hideinitializer
 *
 *  Useful if you only want to rethrow the exception caught, and do not want to
 *  bother with an unused variable.
 */
#define CATCH_ANONYMOUS DO_CATCH(0)

#define DO_CATCH(_xbt_do_catch_set_e) \
            else { \
            } \
            if (!(__ex_cleanup)) \
                __xbt_ex_ctx_ptr->ctx_caught = 0; \
        } else { \
            if (!(__ex_cleanup)) { \
                __ex_mctx_restored(&__ex_mctx_me); \
                __xbt_ex_ctx_ptr->ctx_caught = 1; \
            } \
        } \
        __xbt_ex_ctx_ptr->ctx_mctx = __ex_mctx_en; \
    } \
    if (   !(__xbt_running_ctx_fetch()->ctx_caught) \
        || ((void)(_xbt_do_catch_set_e),                             \
            MAYDAY_CATCH(__xbt_running_ctx_fetch()->exception) 0)) {    \
    } \
    else

#define DO_THROW(running_ctx)                                           \
  do { /* deal with the exception */                                    \
    xbt_running_ctx_t *ctx = (running_ctx);                             \
    if (ctx->ctx_mctx == NULL)                                          \
      __xbt_ex_terminate((xbt_ex_t*)&(ctx->exception)); /* not catched */ \
    else                                                                \
      __ex_mctx_restore(ctx->ctx_mctx); /* catched somewhere */         \
    abort();  /* nope, stupid GCC, we won't survive a THROW */          \
              /* (this won't be reached) */                             \
  } while(0)

/** @brief Helper macro for THROW and THROWF
 *  @hideinitializer
 *
 *  @param _throw_ctx: the throwing context in which we should construct the exception
 *  @param c: category code (integer)
 *  @param v: value (integer)
 *  @param m: message text
 *
 * If called from within a TRY/CATCH construct, this exception 
 * is copied into the CATCH relevant variable program control flow 
 * is derouted to the CATCH (after the optional sg_cleanup).
 *
 * If no TRY/CATCH construct embeds this call, the program calls
 * abort(3). 
 *
 * The THROW can be performed everywhere, including inside TRY, 
 * CLEANUP and CATCH blocks.
 */

#define THROW_PREPARE(_throw_ctx, c, v, m)                              \
  /* build the exception */                                             \
  _throw_ctx->exception.msg      = (m);                                 \
  _throw_ctx->exception.category = (xbt_errcat_t)(c);                   \
  _throw_ctx->exception.value    = (v);                                 \
  _throw_ctx->exception.procname = (char*)xbt_procname();               \
  _throw_ctx->exception.pid      = xbt_getpid();                        \
  _throw_ctx->exception.file     = (char*)__FILE__;                     \
  _throw_ctx->exception.line     = __LINE__;                            \
  _throw_ctx->exception.func     = (char*)_XBT_FUNCTION;                \
  _throw_ctx->exception.bt_strings = NULL;                              \
  xbt_backtrace_current((xbt_ex_t *)&(_throw_ctx->exception));

#define _THROW(c, v, m)                                        \
  do { /* change this sequence into one block */               \
    xbt_running_ctx_t *_throw_ctx = __xbt_running_ctx_fetch(); \
    THROW_PREPARE(_throw_ctx, c, v, m);                        \
    DO_THROW(_throw_ctx);                                      \
  } while (0)

/** @brief Builds and throws an exception
    @hideinitializer */
#define THROW(c, v)             _THROW(c, v, NULL)

/** @brief Builds and throws an exception with a printf-like formatted message
    @hideinitializer */
#define THROWF(c, v, ...)       _THROW(c, v, bprintf(__VA_ARGS__))

#define THROW_IMPOSSIBLE \
  THROWF(unknown_error, 0, "The Impossible Did Happen (yet again)")
#define THROW_UNIMPLEMENTED \
  THROWF(unknown_error, 0, "Function %s unimplemented",_XBT_FUNCTION)

#define DIE_IMPOSSIBLE xbt_die("The Impossible Did Happen (yet again)")

/** @brief re-throwing of an already caught exception (ie, pass it to the upper catch block) 
 *  @hideinitializer
 */
#define RETHROW DO_THROW(__xbt_running_ctx_fetch())

/** @brief like THROWF, but adding some details to the message of an existing exception
 *  @hideinitializer
 */
#define RETHROWF(...)                                                   \
  do {                                                                  \
    char *_xbt_ex_internal_msg = __xbt_running_ctx_fetch()->exception.msg; \
    __xbt_running_ctx_fetch()->exception.msg = bprintf(__VA_ARGS__,     \
                                                       _xbt_ex_internal_msg); \
    free(_xbt_ex_internal_msg);                                         \
    RETHROW;                                                            \
  } while (0)

/** @brief Exception destructor */
XBT_PUBLIC(void) xbt_ex_free(xbt_ex_t e);
/** @brief The display made by an exception that is not catched */
XBT_PUBLIC(void) xbt_ex_display(xbt_ex_t * e);

/** @brief Shows a backtrace of the current location */
XBT_PUBLIC(void) xbt_backtrace_display_current(void);
/** @brief reimplementation of glibc backtrace based directly on gcc library, without implicit malloc  */
XBT_PUBLIC(int) xbt_backtrace_no_malloc(void**bt, int size);
/** @brief Captures a backtrace for further use */
XBT_PUBLIC(void) xbt_backtrace_current(xbt_ex_t * e);
/** @brief Display a previously captured backtrace */
XBT_PUBLIC(void) xbt_backtrace_display(xbt_ex_t * e);
/** @brief Get current backtrace with libunwind */
XBT_PUBLIC(int) xbt_libunwind_backtrace(void *bt[XBT_BACKTRACE_SIZE], int size);

#ifdef XBT_USE_DEPRECATED

/* Kept for backward compatibility. */

#define THROW0(c, v, m) \
  do { if (m) THROWF(c, v, m); else THROW(c, v); } while (0)
#define THROW1(c, v, ...)       THROWF(c, v, __VA_ARGS__)
#define THROW2(c, v, ...)       THROWF(c, v, __VA_ARGS__)
#define THROW3(c, v, ...)       THROWF(c, v, __VA_ARGS__)
#define THROW4(c, v, ...)       THROWF(c, v, __VA_ARGS__)
#define THROW5(c, v, ...)       THROWF(c, v, __VA_ARGS__)
#define THROW6(c, v, ...)       THROWF(c, v, __VA_ARGS__)
#define THROW7(c, v, ...)       THROWF(c, v, __VA_ARGS__)

#define RETHROW0(...)           RETHROWF(__VA_ARGS__)
#define RETHROW1(...)           RETHROWF(__VA_ARGS__)
#define RETHROW2(...)           RETHROWF(__VA_ARGS__)
#define RETHROW3(...)           RETHROWF(__VA_ARGS__)
#define RETHROW4(...)           RETHROWF(__VA_ARGS__)
#define RETHROW5(...)           RETHROWF(__VA_ARGS__)

#endif

SG_END_DECL()

/** @} */
#endif                          /* __XBT_EX_H__ */
