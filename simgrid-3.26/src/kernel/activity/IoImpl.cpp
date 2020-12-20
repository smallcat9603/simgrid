/* Copyright (c) 2007-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */
#include "src/kernel/activity/IoImpl.hpp"
#include "mc/mc.h"
#include "simgrid/Exception.hpp"
#include "simgrid/kernel/resource/Action.hpp"
#include "simgrid/s4u/Host.hpp"
#include "src/kernel/resource/DiskImpl.hpp"
#include "src/mc/mc_replay.hpp"
#include "src/simix/smx_private.hpp"
#include "src/surf/StorageImpl.hpp"
#include "src/surf/cpu_interface.hpp"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(simix_io, simix, "Logging specific to SIMIX (io)");

namespace simgrid {
namespace kernel {
namespace activity {

IoImpl& IoImpl::set_timeout(double timeout)
{
  const s4u::Host* host = get_disk() ? get_disk()->get_host() : s4u::Host::by_name(get_storage()->get_host());
  timeout_detector_ = host->pimpl_cpu->sleep(timeout);
  timeout_detector_->set_activity(this);
  return *this;
}

IoImpl& IoImpl::set_type(s4u::Io::OpType type)
{
  type_ = type;
  return *this;
}

IoImpl& IoImpl::set_size(sg_size_t size)
{
  size_ = size;
  return *this;
}

IoImpl& IoImpl::set_disk(resource::DiskImpl* disk)
{
  disk_ = disk;
  return *this;
}

IoImpl& IoImpl::set_storage(resource::StorageImpl* storage)
{
  storage_ = storage;
  return *this;
}

IoImpl* IoImpl::start()
{
  state_ = State::RUNNING;
  if (storage_)
    surf_action_ = storage_->io_start(size_, type_);
  else
    surf_action_ = disk_->io_start(size_, type_);
  surf_action_->set_activity(this);

  XBT_DEBUG("Create IO synchro %p %s", this, get_cname());
  IoImpl::on_start(*this);

  return this;
}

void IoImpl::post()
{
  performed_ioops_ = surf_action_->get_cost();
  if (surf_action_->get_state() == resource::Action::State::FAILED) {
    if ((storage_ && not storage_->is_on()) || (disk_ && not disk_->is_on()))
      state_ = State::FAILED;
    else
      state_ = State::CANCELED;
  } else if (surf_action_->get_state() == resource::Action::State::FINISHED) {
    state_ = State::DONE;
  } else if (timeout_detector_ && timeout_detector_->get_state() == resource::Action::State::FINISHED) {
    state_ = State::TIMEOUT;
  }

  clean_action();
  if (timeout_detector_) {
    timeout_detector_->unref();
    timeout_detector_ = nullptr;
  }

  on_completion(*this);

  /* Answer all simcalls associated with the synchro */
  finish();
}

void IoImpl::finish()
{
  while (not simcalls_.empty()) {
    const s_smx_simcall* simcall = simcalls_.front();
    simcalls_.pop_front();
    switch (state_) {
      case State::DONE:
        /* do nothing, synchro done */
        break;
      case State::FAILED:
        simcall->issuer_->context_->set_wannadie();
        simcall->issuer_->exception_ =
            std::make_exception_ptr(StorageFailureException(XBT_THROW_POINT, "Storage failed"));
        break;
      case State::CANCELED:
        simcall->issuer_->exception_ = std::make_exception_ptr(CancelException(XBT_THROW_POINT, "I/O Canceled"));
        break;
      case State::TIMEOUT:
        XBT_DEBUG("IoImpl::finish(): execution timeouted");
        simcall->issuer_->exception_ = std::make_exception_ptr(simgrid::TimeoutException(XBT_THROW_POINT, "Timeouted"));
        break;
      default:
        xbt_die("Internal error in IoImpl::finish(): unexpected synchro state %d", static_cast<int>(state_));
    }

    simcall->issuer_->waiting_synchro_ = nullptr;
    simcall->issuer_->simcall_answer();
  }
}

/*************
 * Callbacks *
 *************/
xbt::signal<void(IoImpl const&)> IoImpl::on_start;
xbt::signal<void(IoImpl const&)> IoImpl::on_completion;

} // namespace activity
} // namespace kernel
} // namespace simgrid
