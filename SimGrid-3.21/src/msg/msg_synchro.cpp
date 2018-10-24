/* Copyright (c) 2013-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "msg_private.hpp"
#include "simgrid/Exception.hpp"
#include "src/simix/smx_private.hpp"
#include "src/simix/smx_synchro_private.hpp"
#include "xbt/synchro.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(msg_synchro, msg, "Logging specific to MSG (synchro)");

/** @addtogroup msg_synchro
 *
 *  @{
 */

/** @brief creates a semaphore object of the given initial capacity */
msg_sem_t MSG_sem_init(int initial_value) {
  return simgrid::simix::simcall([initial_value] { return SIMIX_sem_init(initial_value); });
}

/** @brief locks on a semaphore object */
void MSG_sem_acquire(msg_sem_t sem) {
  simcall_sem_acquire(sem);
}

/** @brief locks on a semaphore object up until the provided timeout expires */
msg_error_t MSG_sem_acquire_timeout(msg_sem_t sem, double timeout) {
  return simcall_sem_acquire_timeout(sem, timeout) ? MSG_TIMEOUT : MSG_OK;
}

/** @brief releases the semaphore object */
void MSG_sem_release(msg_sem_t sem) {
  simgrid::simix::simcall([sem] { SIMIX_sem_release(sem); });
}

int MSG_sem_get_capacity(msg_sem_t sem) {
  return simgrid::simix::simcall([sem] { return SIMIX_sem_get_capacity(sem); });
}

void MSG_sem_destroy(msg_sem_t sem) {
  SIMIX_sem_destroy(sem);
}

/** @brief returns a boolean indicating if this semaphore would block at this very specific time
 *
 * Note that the returned value may be wrong right after the function call, when you try to use it...
 * But that's a classical semaphore issue, and SimGrid's semaphore are not different to usual ones here.
 */
int MSG_sem_would_block(msg_sem_t sem) {
  return simgrid::simix::simcall([sem] { return SIMIX_sem_would_block(sem); });
}

/**@}*/
