/* Copyright (c) 2006-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/forward.h"
#include "simgrid/mutex.h"
#include "simgrid/s4u/Mutex.hpp"
#include "src/kernel/activity/MutexImpl.hpp"

namespace simgrid {
namespace s4u {

Mutex::~Mutex()
{
  if (pimpl_ != nullptr)
    pimpl_->unref();
}

/** @brief Blocks the calling actor until the mutex can be obtained */
void Mutex::lock()
{
  simcall_mutex_lock(pimpl_);
}

/** @brief Release the ownership of the mutex, unleashing a blocked actor (if any)
 *
 * Will fail if the calling actor does not own the mutex.
 */
void Mutex::unlock()
{
  simcall_mutex_unlock(pimpl_);
}

/** @brief Acquire the mutex if it's free, and return false (without blocking) if not */
bool Mutex::try_lock()
{
  return simcall_mutex_trylock(pimpl_);
}

/** @brief Create a new mutex
 *
 * See @ref s4u_raii.
 */
MutexPtr Mutex::create()
{
  kernel::activity::MutexImpl* mutex = kernel::actor::simcall([] { return new kernel::activity::MutexImpl(); });
  return MutexPtr(&mutex->mutex(), false);
}

/* refcounting of the intrusive_ptr is delegated to the implementation object */
void intrusive_ptr_add_ref(const Mutex* mutex)
{
  xbt_assert(mutex);
  if (mutex->pimpl_)
    mutex->pimpl_->ref();
}
void intrusive_ptr_release(const Mutex* mutex)
{
  xbt_assert(mutex);
  if (mutex->pimpl_)
    mutex->pimpl_->unref();
}

} // namespace s4u
} // namespace simgrid

/* **************************** Public C interface *************************** */
sg_mutex_t sg_mutex_init()
{
  simgrid::kernel::activity::MutexImpl* mutex =
      simgrid::kernel::actor::simcall([] { return new simgrid::kernel::activity::MutexImpl(); });

  return new simgrid::s4u::Mutex(mutex);
}

void sg_mutex_lock(sg_mutex_t mutex)
{
  mutex->lock();
}

void sg_mutex_unlock(sg_mutex_t mutex)
{
  mutex->unlock();
}

int sg_mutex_try_lock(sg_mutex_t mutex)
{
  return mutex->try_lock();
}

void sg_mutex_destroy(const_sg_mutex_t mutex)
{
  delete mutex;
}
