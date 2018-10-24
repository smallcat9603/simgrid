/* Copyright (c) 2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_S4U_BARRIER_HPP
#define SIMGRID_S4U_BARRIER_HPP

#include <simgrid/forward.h>
#include <simgrid/s4u/ConditionVariable.hpp>
#include <simgrid/chrono.hpp>
#include <simgrid/s4u/Mutex.hpp>

#include <atomic>
#include <future>

namespace simgrid {
namespace s4u {

class XBT_PUBLIC Barrier {
private:
  MutexPtr mutex_;
  ConditionVariablePtr cond_;
  unsigned int expected_processes_;
  unsigned int arrived_processes_ = 0;

  /* refcounting */
  std::atomic_int_fast32_t refcount_{0};

public:
  explicit Barrier(unsigned int count);
  ~Barrier()              = default;
  Barrier(Barrier const&) = delete;
  Barrier& operator=(Barrier const&) = delete;

  /** Constructs a new barrier */
  static BarrierPtr create(unsigned int expected_processes);

  int wait();

  /* refcounting */
  friend XBT_PUBLIC void intrusive_ptr_add_ref(Barrier* barrier);
  friend XBT_PUBLIC void intrusive_ptr_release(Barrier* barrier);
};
}
} // namespace simgrid::s4u

#endif
