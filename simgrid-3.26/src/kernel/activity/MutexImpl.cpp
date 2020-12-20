/* Copyright (c) 2007-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "src/kernel/activity/MutexImpl.hpp"
#include "src/kernel/activity/SynchroRaw.hpp"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(simix_mutex, simix_synchro, "Mutex kernel-space implementation");

namespace simgrid {
namespace kernel {
namespace activity {

void MutexImpl::lock(actor::ActorImpl* issuer)
{
  XBT_IN("(%p; %p)", this, issuer);
  /* FIXME: check where to validate the arguments */
  RawImplPtr synchro = nullptr;

  if (locked_) {
    /* FIXME: check if the host is active ? */
    /* Somebody using the mutex, use a synchronization to get host failures */
    synchro = RawImplPtr(new RawImpl());
    (*synchro).set_host(issuer->get_host()).start();
    synchro->simcalls_.push_back(&issuer->simcall_);
    issuer->waiting_synchro_ = synchro;
    sleeping_.push_back(*issuer);
  } else {
    /* mutex free */
    locked_ = true;
    owner_  = issuer;
    issuer->simcall_answer();
  }
  XBT_OUT();
}

/** Tries to lock the mutex for a process
 *
 * @param  issuer  the process that tries to acquire the mutex
 * @return whether we managed to lock the mutex
 */
bool MutexImpl::try_lock(actor::ActorImpl* issuer)
{
  XBT_IN("(%p, %p)", this, issuer);
  if (locked_) {
    XBT_OUT();
    return false;
  }

  locked_ = true;
  owner_  = issuer;
  XBT_OUT();
  return true;
}

/** Unlock a mutex for a process
 *
 * Unlocks the mutex and gives it to a process waiting for it.
 * If the unlocker is not the owner of the mutex nothing happens.
 * If there are no process waiting, it sets the mutex as free.
 */
void MutexImpl::unlock(actor::ActorImpl* issuer)
{
  XBT_IN("(%p, %p)", this, issuer);
  xbt_assert(locked_, "Cannot release that mutex: it was not locked.");
  xbt_assert(issuer == owner_, "Cannot release that mutex: it was locked by %s (pid:%ld), not by you.",
             owner_->get_cname(), owner_->get_pid());

  if (not sleeping_.empty()) {
    /* Give the ownership to the first waiting actor */
    owner_ = &sleeping_.front();
    sleeping_.pop_front();
    owner_->waiting_synchro_ = nullptr;
    owner_->simcall_answer();
  } else {
    /* nobody to wake up */
    locked_ = false;
    owner_  = nullptr;
  }
  XBT_OUT();
}
/** Increase the refcount for this mutex */
MutexImpl* MutexImpl::ref()
{
  intrusive_ptr_add_ref(this);
  return this;
}

/** Decrease the refcount for this mutex */
void MutexImpl::unref()
{
  intrusive_ptr_release(this);
}

} // namespace activity
} // namespace kernel
} // namespace simgrid

// Simcall handlers:

void simcall_HANDLER_mutex_lock(smx_simcall_t simcall, smx_mutex_t mutex)
{
  mutex->lock(simcall->issuer_);
}

int simcall_HANDLER_mutex_trylock(smx_simcall_t simcall, smx_mutex_t mutex)
{
  return mutex->try_lock(simcall->issuer_);
}

void simcall_HANDLER_mutex_unlock(smx_simcall_t simcall, smx_mutex_t mutex)
{
  mutex->unlock(simcall->issuer_);
}
