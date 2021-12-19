/* xbt_synchro -- Synchronization virtualized depending on whether we are   */
/*                in simulation or real life (act on simulated processes)   */

/* This is the simulation implementation, using simix.                      */

/* Copyright (c) 2007-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt/ex.h"
#include "xbt/synchro_core.h"

#include "simgrid/simix.h"        /* used implementation */
#include "../simix/smx_private.h" /* FIXME */

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(xbt_sync, xbt,
                                "Synchronization mechanism");

/* the implementation would be cleaner (and faster) with ELF symbol aliasing */

typedef struct s_xbt_thread_ {
  smx_process_t s_process; /* keep this first, gras_socket_im_the_server() does funky transtyping in sg_msg.c */
  char *name;
  void_f_pvoid_t code;
  void *userparam;
  void *father_data;
  /* stuff to allow other people to wait on me with xbt_thread_join */
  unsigned joinable:1, done:1;
  xbt_cond_t cond;
  xbt_mutex_t mutex;
} s_xbt_thread_t;

static int xbt_thread_create_wrapper(int argc, char *argv[])
{
  smx_process_t self = SIMIX_process_self();
  xbt_thread_t t =
      (xbt_thread_t) SIMIX_process_self_get_data(self);
  simcall_process_set_data(self, t->father_data);
  t->code(t->userparam);
  if (t->joinable) {
    t->done = 1;
    xbt_mutex_acquire(t->mutex);
    xbt_cond_broadcast(t->cond);
    xbt_mutex_release(t->mutex);
  } else {
    xbt_mutex_destroy(t->mutex);
    xbt_cond_destroy(t->cond);
    free(t->name);
    free(t);
  }
  return 0;
}

xbt_thread_t xbt_thread_create(const char *name, void_f_pvoid_t code,
                               void *param, int joinable)
{
  xbt_thread_t res = xbt_new0(s_xbt_thread_t, 1);
  res->name = xbt_strdup(name);
  res->userparam = param;
  res->code = code;
  res->father_data = SIMIX_process_self_get_data(SIMIX_process_self());
  /*   char*name = bprintf("%s#%p",SIMIX_process_self_get_name(), param); */
  simcall_process_create(&res->s_process, name,
                           xbt_thread_create_wrapper, res,
                           SIMIX_host_self_get_name(), -1.0, 0, NULL,
                           /*props */ NULL,0);
  res->joinable = joinable;
  res->done = 0;
  res->cond = xbt_cond_init();
  res->mutex = xbt_mutex_init();
  //   free(name);
  return res;
}

const char *xbt_thread_name(xbt_thread_t t)
{
  return t->name;
}

const char *xbt_thread_self_name(void)
{
  xbt_thread_t me = xbt_thread_self();
  return me ? me->name : "maestro";
}


void xbt_thread_join(xbt_thread_t thread)
{
  xbt_mutex_acquire(thread->mutex);
  xbt_assert(thread->joinable,
              "Cannot join on %p: wasn't created joinable", thread);
  if (!thread->done) {
    xbt_cond_wait(thread->cond, thread->mutex);
    xbt_mutex_release(thread->mutex);
  }

  xbt_mutex_destroy(thread->mutex);
  xbt_cond_destroy(thread->cond);
  free(thread->name);
  free(thread);

}

void xbt_thread_cancel(xbt_thread_t thread)
{
  simcall_process_kill(thread->s_process);
  free(thread->name);
  free(thread);
}

void xbt_thread_exit()
{
  simcall_process_kill(SIMIX_process_self());
}

xbt_thread_t xbt_thread_self(void)
{
  return SIMIX_process_self_get_data(SIMIX_process_self());
}

void xbt_thread_yield(void)
{
  SIMIX_process_yield(SIMIX_process_self());
}

/****** mutex related functions ******/
struct s_xbt_mutex_ {
  s_smx_mutex_t mutex;
};

xbt_mutex_t xbt_mutex_init(void)
{
  return (xbt_mutex_t) simcall_mutex_init();
}

void xbt_mutex_acquire(xbt_mutex_t mutex)
{
  simcall_mutex_lock((smx_mutex_t) mutex);
}

int xbt_mutex_try_acquire(xbt_mutex_t mutex)
{
  return simcall_mutex_trylock((smx_mutex_t) mutex);
}

void xbt_mutex_release(xbt_mutex_t mutex)
{
  simcall_mutex_unlock((smx_mutex_t) mutex);
}

void xbt_mutex_destroy(xbt_mutex_t mutex)
{
  simcall_mutex_destroy((smx_mutex_t) mutex);
}

/***** condition related functions *****/
struct s_xbt_cond_ {
  s_smx_cond_t cond;
};

xbt_cond_t xbt_cond_init(void)
{
  return (xbt_cond_t) simcall_cond_init();
}

void xbt_cond_wait(xbt_cond_t cond, xbt_mutex_t mutex)
{
  simcall_cond_wait((smx_cond_t) cond, (smx_mutex_t) mutex);
}

void xbt_cond_timedwait(xbt_cond_t cond, xbt_mutex_t mutex, double delay)
{
  simcall_cond_wait_timeout((smx_cond_t) cond, (smx_mutex_t) mutex, delay);
}

void xbt_cond_signal(xbt_cond_t cond)
{
  simcall_cond_signal((smx_cond_t) cond);
}

void xbt_cond_broadcast(xbt_cond_t cond)
{
  simcall_cond_broadcast((smx_cond_t) cond);
}

void xbt_cond_destroy(xbt_cond_t cond)
{
  simcall_cond_destroy((smx_cond_t) cond);
}

/***** barrier related functions *****/
typedef struct s_xbt_bar_ {
  xbt_mutex_t mutex;
  xbt_cond_t cond;
  unsigned int arrived_processes;
  unsigned int expected_processes;
} s_xbt_bar_;

xbt_bar_t xbt_barrier_init(unsigned int count)
{
  xbt_bar_t bar = xbt_new0(s_xbt_bar_, 1);
  bar->expected_processes = count;
  bar->arrived_processes = 0;
  bar->mutex = xbt_mutex_init();
  bar->cond = xbt_cond_init();
  return bar;
}


int xbt_barrier_wait(xbt_bar_t bar)
{
   int ret=0;
   xbt_mutex_acquire(bar->mutex);
   if (++bar->arrived_processes == bar->expected_processes) {
     xbt_cond_broadcast(bar->cond);
     xbt_mutex_release(bar->mutex);
     ret=XBT_BARRIER_SERIAL_PROCESS;
     bar->arrived_processes = 0;
   } else {
     xbt_cond_wait(bar->cond, bar->mutex);
     xbt_mutex_release(bar->mutex);
   }

   return ret;
}

void xbt_barrier_destroy(xbt_bar_t bar)
{
   xbt_mutex_destroy(bar->mutex);
   xbt_cond_destroy(bar->cond);
   xbt_free(bar);
}

