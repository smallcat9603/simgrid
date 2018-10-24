/* xbt_os_thread -- portability layer over the pthread API                  */
/* Used in RL to get win/lin portability, and in SG when CONTEXT_THREAD     */
/* in SG, when using HAVE_UCONTEXT_CONTEXTS, xbt_os_thread_stub is used instead   */

/* Copyright (c) 2007-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "src/internal_config.h"
#if HAVE_PTHREAD_SETAFFINITY
#define _GNU_SOURCE
#include <sched.h>
#endif

#include <pthread.h>

#if defined(__FreeBSD__)
#include "pthread_np.h"
#define cpu_set_t cpuset_t
#endif

#include <limits.h>
#include <semaphore.h>
#include <errno.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__MACH__) && defined(__APPLE__)
#include <stdint.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif

#include "xbt/sysdep.h"
#include "xbt/ex.h"
#include "src/internal_config.h"
#include "xbt/xbt_os_time.h"       /* Portable time facilities */
#include "xbt/xbt_os_thread.h"     /* This module */
#include "src/xbt_modinter.h"      /* Initialization/finalization of this module */

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(xbt_sync_os, xbt, "Synchronization mechanism (OS-level)");

/* use named semaphore when sem_init() does not work */
#if !HAVE_SEM_INIT
static int next_sem_ID = 0;
static xbt_os_mutex_t next_sem_ID_lock;
#endif

typedef struct xbt_os_thread_ {
  pthread_t t;
  char *name;
  void *param;
  pvoid_f_pvoid_t start_routine;
  void *extra_data;
} s_xbt_os_thread_t;
static xbt_os_thread_t main_thread = NULL;

/* thread-specific data containing the xbt_os_thread_t structure */
static pthread_key_t xbt_self_thread_key;
static int thread_mod_inited = 0;

/* defaults attribute for pthreads */
//FIXME: find where to put this
static pthread_attr_t thread_attr;

/* frees the xbt_os_thread_t corresponding to the current thread */
static void xbt_os_thread_free_thread_data(xbt_os_thread_t thread)
{
  if (thread == main_thread)    /* just killed main thread */
    main_thread = NULL;
  free(thread->name);
  free(thread);
}

void xbt_os_thread_mod_preinit(void)
{
  if (thread_mod_inited)
    return;

  int errcode = pthread_key_create(&xbt_self_thread_key, NULL);
  xbt_assert(errcode == 0, "pthread_key_create failed for xbt_self_thread_key");

  main_thread = xbt_new(s_xbt_os_thread_t, 1);
  main_thread->name = NULL;
  main_thread->name = xbt_strdup("main");
  main_thread->param = NULL;
  main_thread->start_routine = NULL;
  main_thread->extra_data = NULL;

  if ((errcode = pthread_setspecific(xbt_self_thread_key, main_thread)))
    THROWF(system_error, errcode,
           "Impossible to set the SimGrid identity descriptor to the main thread (pthread_setspecific failed)");

  pthread_attr_init(&thread_attr);

  thread_mod_inited = 1;

#if !HAVE_SEM_INIT
  next_sem_ID_lock = xbt_os_mutex_init();
#endif
}

void xbt_os_thread_mod_postexit(void)
{
  /* FIXME: don't try to free our key on shutdown.
     Valgrind detects no leak if we don't, and whine if we try to */
  //   int errcode;

  //   if ((errcode=pthread_key_delete(xbt_self_thread_key)))
  //     THROWF(system_error,errcode,"pthread_key_delete failed for xbt_self_thread_key");
  free(main_thread->name);
  free(main_thread);
  main_thread = NULL;
  thread_mod_inited = 0;
#if !HAVE_SEM_INIT
  xbt_os_mutex_destroy(next_sem_ID_lock);
#endif
}

/** Calls pthread_atfork() if present, and raise an exception otherwise.
 *
 * The only known user of this wrapper is mmalloc_preinit(), but it is absolutely mandatory there:
 * when used with tesh, mmalloc *must* be mutex protected and resistant to forks.
 * This functionality is the only way to get it working (by ensuring that the mutex is consistently released on forks)
 */

/* this function is critical to tesh+mmalloc, don't mess with it */
int xbt_os_thread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
  return pthread_atfork(prepare, parent, child);
}

static void *wrapper_start_routine(void *s)
{
  xbt_os_thread_t t = s;

  int errcode = pthread_setspecific(xbt_self_thread_key, t);
  xbt_assert(errcode == 0, "pthread_setspecific failed for xbt_self_thread_key");

  return t->start_routine(t->param);
}

xbt_os_thread_t xbt_os_thread_create(const char *name,  pvoid_f_pvoid_t start_routine, void *param, void *extra_data)
{
  xbt_os_thread_t res_thread = xbt_new(s_xbt_os_thread_t, 1);
  res_thread->name = xbt_strdup(name);
  res_thread->start_routine = start_routine;
  res_thread->param = param;
  res_thread->extra_data = extra_data;

  int errcode = pthread_create(&(res_thread->t), &thread_attr, wrapper_start_routine, res_thread);
  xbt_assert(errcode == 0, "pthread_create failed: %s", strerror(errcode));

  return res_thread;
}

/** Bind the thread to the given core, if possible.
 *
 * If pthread_setaffinity_np is not usable on that (non-gnu) platform, this function does nothing.
 */
int xbt_os_thread_bind(XBT_ATTRIB_UNUSED xbt_os_thread_t thread, XBT_ATTRIB_UNUSED int cpu)
{
  int errcode = 0;
#if HAVE_PTHREAD_SETAFFINITY
  pthread_t pthread = thread->t;
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu, &cpuset);
  errcode = pthread_setaffinity_np(pthread, sizeof(cpu_set_t), &cpuset);
#endif
  return errcode;
}

void xbt_os_thread_setstacksize(int stack_size)
{
  size_t alignment[] = {
    xbt_pagesize,
#ifdef PTHREAD_STACK_MIN
    PTHREAD_STACK_MIN,
#endif
    0
  };

  xbt_assert(stack_size >= 0, "stack size %d is negative, maybe it exceeds MAX_INT?", stack_size);

  size_t sz = stack_size;
  int res = pthread_attr_setstacksize(&thread_attr, sz);

  for (int i = 0; res == EINVAL && alignment[i] > 0; i++) {
    /* Invalid size, try again with next multiple of alignment[i]. */
    size_t rem = sz % alignment[i];
    if (rem != 0 || sz == 0) {
      size_t sz2 = sz - rem + alignment[i];
      XBT_DEBUG("pthread_attr_setstacksize failed for %zu, try again with %zu", sz, sz2);
      sz = sz2;
      res = pthread_attr_setstacksize(&thread_attr, sz);
    }
  }

  if (res == EINVAL)
    XBT_WARN("invalid stack size (maybe too big): %zu", sz);
  else if (res != 0)
    XBT_WARN("unknown error %d in pthread stacksize setting: %zu", res, sz);
}

void xbt_os_thread_setguardsize(int guard_size)
{
#ifdef WIN32
  THROW_UNIMPLEMENTED; //pthread_attr_setguardsize is not implemented in pthread.h on windows
#else
  size_t sz = guard_size;
  int res = pthread_attr_setguardsize(&thread_attr, sz);
  if (res)
    XBT_WARN("pthread_attr_setguardsize failed (%d) for size: %zu", res, sz);
#endif
}

const char *xbt_os_thread_self_name(void)
{
  xbt_os_thread_t me = xbt_os_thread_self();
  return me ? (const char *)me->name : "main";
}

void xbt_os_thread_join(xbt_os_thread_t thread, void **thread_return)
{
  int errcode = pthread_join(thread->t, thread_return);

  xbt_assert(errcode==0, "pthread_join failed: %s", strerror(errcode));
  xbt_os_thread_free_thread_data(thread);
}

void xbt_os_thread_exit(int *retval)
{
  pthread_exit(retval);
}

xbt_os_thread_t xbt_os_thread_self(void )
{
  if (!thread_mod_inited)
    return NULL;

  return pthread_getspecific(xbt_self_thread_key);
}

/****** mutex related functions ******/
typedef struct xbt_os_mutex_ {
  pthread_mutex_t m;
} s_xbt_os_mutex_t;

#include <time.h>
#include <math.h>

xbt_os_mutex_t xbt_os_mutex_init(void)
{
  pthread_mutexattr_t Attr;
  pthread_mutexattr_init(&Attr);
  pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);

  xbt_os_mutex_t res = xbt_new(s_xbt_os_mutex_t, 1);
  int errcode = pthread_mutex_init(&(res->m), &Attr);
  xbt_assert(errcode==0, "pthread_mutex_init() failed: %s", strerror(errcode));

  return res;
}

void xbt_os_mutex_acquire(xbt_os_mutex_t mutex)
{
  int errcode = pthread_mutex_lock(&(mutex->m));
  xbt_assert(errcode==0, "pthread_mutex_lock(%p) failed: %s", mutex, strerror(errcode));
}

void xbt_os_mutex_release(xbt_os_mutex_t mutex)
{
  int errcode = pthread_mutex_unlock(&(mutex->m));
  xbt_assert(errcode==0, "pthread_mutex_unlock(%p) failed: %s", mutex, strerror(errcode));
}

void xbt_os_mutex_destroy(xbt_os_mutex_t mutex)
{
  if (!mutex)
    return;

  int errcode = pthread_mutex_destroy(&(mutex->m));
  xbt_assert(errcode == 0, "pthread_mutex_destroy(%p) failed: %s", mutex, strerror(errcode));
  free(mutex);
}

typedef struct xbt_os_sem_ {
#if !HAVE_SEM_INIT
  char *name;
#endif
  sem_t s;
  sem_t *ps;
} s_xbt_os_sem_t;

#ifndef SEM_FAILED
#define SEM_FAILED (-1)
#endif

xbt_os_sem_t xbt_os_sem_init(unsigned int value)
{
  xbt_os_sem_t res = xbt_new(s_xbt_os_sem_t, 1);

  /* On some systems (MAC OS X), only the stub of sem_init is to be found.
   * Any attempt to use it leads to ENOSYS (function not implemented).
   * If such a prehistoric system is detected, do the job with sem_open instead
   */
#if HAVE_SEM_INIT
  if (sem_init(&(res->s), 0, value) != 0)
    THROWF(system_error, errno, "sem_init() failed: %s", strerror(errno));
  res->ps = &(res->s);

#else                           /* damn, no sem_init(). Reimplement it */

  xbt_os_mutex_acquire(next_sem_ID_lock);
  res->name = bprintf("/sg-%d", ++next_sem_ID);
  xbt_os_mutex_release(next_sem_ID_lock);

  sem_unlink(res->name);
  res->ps = sem_open(res->name, O_CREAT, 0644, value);
  if ((res->ps == (sem_t *) SEM_FAILED) && (errno == ENAMETOOLONG)) {
    /* Old darwins only allow 13 chars. Did you create *that* amount of semaphores? */
    res->name[13] = '\0';
    sem_unlink(res->name);
    res->ps = sem_open(res->name, O_CREAT, 0644, value);
  }
  if (res->ps == (sem_t *) SEM_FAILED)
    THROWF(system_error, errno, "sem_open() failed: %s", strerror(errno));

  /* Remove the name from the semaphore namespace: we never join on it */
  if (sem_unlink(res->name) < 0)
    THROWF(system_error, errno, "sem_unlink() failed: %s",
           strerror(errno));

#endif

  return res;
}

void xbt_os_sem_acquire(xbt_os_sem_t sem)
{
  if (sem_wait(sem->ps) < 0)
    THROWF(system_error, errno, "sem_wait() failed: %s", strerror(errno));
}

void xbt_os_sem_release(xbt_os_sem_t sem)
{
  if (sem_post(sem->ps) < 0)
    THROWF(system_error, errno, "sem_post() failed: %s", strerror(errno));
}

void xbt_os_sem_destroy(xbt_os_sem_t sem)
{
#if HAVE_SEM_INIT
  if (sem_destroy(sem->ps) < 0)
    THROWF(system_error, errno, "sem_destroy() failed: %s", strerror(errno));
#else
  if (sem_close(sem->ps) < 0)
    THROWF(system_error, errno, "sem_close() failed: %s", strerror(errno));
  xbt_free(sem->name);
#endif
  xbt_free(sem);
}

void xbt_os_thread_set_extra_data(void *data)
{
  xbt_os_thread_self()->extra_data = data;
}

void *xbt_os_thread_get_extra_data(void)
{
  xbt_os_thread_t thread = xbt_os_thread_self();
  return thread ? thread->extra_data : NULL;
}
