/* xbt_sg_stubs -- empty functions sometimes used in SG (never in RL)       */

/* This is always part of SG, never of RL. Content:                         */
/*  - a bunch of stub functions of the thread related function that we need */
/*    to add to the lib to please the linker when using ucontexts.          */
/*  - a bunch of stub functions of the java related function when we don't  */
/*    compile java bindings.                                                */

/* In RL, java is useless, and threads are always part of the picture,      */
/*  ucontext never */

/* Copyright (c) 2007-2010, 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt_modinter.h"
#include "xbt/sysdep.h"
#include "xbt/xbt_os_thread.h"
#include "portable.h"           /* CONTEXT_THREADS */

#ifndef CONTEXT_THREADS
#ifndef WIN32
#ifdef HAVE_PTHREAD_H
/* xbt_threads is loaded in libsimgrid when they are used to implement the xbt_context.
 * The decision (and the loading) is made in xbt/context.c.
 */

int xbt_os_thread_atfork(void (*prepare)(void),
                         void (*parent)(void), void (*child)(void))
{
  return 0;
}

/* Mod_init/exit mecanism */
void xbt_os_thread_mod_preinit(void)
{
}

void xbt_os_thread_mod_postexit(void)
{
}


/* Main functions */

xbt_os_thread_t xbt_os_thread_create(const char *name,
                                     pvoid_f_pvoid_t start_routine,
                                     void *param, void *data)
{
  xbt_backtrace_display_current();
  xbt_die
      ("No pthread in SG when compiled against the ucontext (xbt_os_thread_create)");
}

void xbt_os_thread_exit(int *retcode)
{
  xbt_backtrace_display_current();
  xbt_die
      ("No pthread in SG when compiled against the ucontext (xbt_os_thread_exit)");
}

void xbt_os_thread_detach(xbt_os_thread_t worker)
{
  xbt_backtrace_display_current();
  xbt_die
      ("No pthread in SG when compiled against the ucontext (xbt_os_thread_exit)");
}

xbt_os_thread_t xbt_os_thread_self(void)
{
  xbt_backtrace_display_current();
  xbt_die
      ("No pthread in SG when compiled against the ucontext (xbt_os_thread_self)");
}

void xbt_os_thread_yield(void)
{
  xbt_backtrace_display_current();
  xbt_die
      ("No pthread in SG when compiled against the ucontext (xbt_os_thread_yield)");
}


xbt_os_mutex_t xbt_os_mutex_init(void)
{
  /*
     xbt_backtrace_display_current();
     xbt_die
     ("No pthread in SG when compiled against the ucontext (xbt_os_mutex_init)");
   */
  return NULL;
}

void xbt_os_mutex_acquire(xbt_os_mutex_t mutex)
{
  /*
     xbt_backtrace_display_current();
     xbt_die
     ("No pthread in SG when compiled against the ucontext (xbt_os_mutex_acquire)");
   */
}

void xbt_os_mutex_release(xbt_os_mutex_t mutex)
{
  /*
     xbt_backtrace_display_current();
     xbt_die
     ("No pthread in SG when compiled against the ucontext (xbt_os_mutex_release)");
   */
}

void xbt_os_mutex_destroy(xbt_os_mutex_t mutex)
{
  /*
     xbt_backtrace_display_current();
     xbt_die
     ("No pthread in SG when compiled against the ucontext (xbt_os_mutex_destroy)");
   */
}

xbt_os_cond_t xbt_os_cond_init(void)
{
  xbt_backtrace_display_current();
  xbt_die
      ("No pthread in SG when compiled against the ucontext (xbt_os_cond_init)");
}

void xbt_os_cond_wait(xbt_os_cond_t cond, xbt_os_mutex_t mutex)
{
  xbt_backtrace_display_current();
  xbt_die
      ("No pthread in SG when compiled against the ucontext (xbt_os_cond_wait)");
}

void xbt_os_cond_signal(xbt_os_cond_t cond)
{
  xbt_backtrace_display_current();
  xbt_die
      ("No pthread in SG when compiled against the ucontext (xbt_os_cond_signal)");
}

void xbt_os_cond_broadcast(xbt_os_cond_t cond)
{
  xbt_backtrace_display_current();
  xbt_die
      ("No pthread in SG when compiled against the ucontext (xbt_os_cond_broadcast)");
}

void xbt_os_cond_destroy(xbt_os_cond_t cond)
{
  xbt_backtrace_display_current();
  xbt_die
      ("No pthread in SG when compiled against the ucontext (xbt_os_cond_destroy)");
}
#endif
#endif
#endif
