/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_KERNEL_ACTIVITY_ACTIVITYIMPL_HPP
#define SIMGRID_KERNEL_ACTIVITY_ACTIVITYIMPL_HPP

#include <string>
#include <list>

#include <xbt/base.h>
#include "simgrid/forward.h"

#include <atomic>
#include <simgrid/simix.hpp>

namespace simgrid {
namespace kernel {
namespace activity {

class XBT_PUBLIC ActivityImpl {
public:
  ActivityImpl();
  explicit ActivityImpl(std::string name) : name_(name) {}
  virtual ~ActivityImpl();
  e_smx_state_t state_ = SIMIX_WAITING; /* State of the activity */
  std::string name_;                    /* Activity name if any */
  std::list<smx_simcall_t> simcalls_;   /* List of simcalls waiting for this activity */

  virtual void suspend() = 0;
  virtual void resume()  = 0;
  virtual void post()    = 0; // What to do when a simcall terminates

  // boost::intrusive_ptr<ActivityImpl> support:
  friend XBT_PUBLIC void intrusive_ptr_add_ref(ActivityImpl* activity);
  friend XBT_PUBLIC void intrusive_ptr_release(ActivityImpl* activity);

private:
  std::atomic_int_fast32_t refcount_{0};
};
}}} // namespace simgrid::kernel::activity

#endif /* SIMGRID_KERNEL_ACTIVITY_ACTIVITYIMPL_HPP */
