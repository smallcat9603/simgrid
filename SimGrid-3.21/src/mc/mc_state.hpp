/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_MC_STATE_HPP
#define SIMGRID_MC_STATE_HPP

#include <list>
#include <memory>

#include "src/mc/mc_record.hpp"
#include "src/mc/sosp/mc_snapshot.hpp"

#include "src/kernel/activity/CommImpl.hpp"
#include "src/mc/Transition.hpp"

namespace simgrid {
namespace mc {

enum class PatternCommunicationType {
  none    = 0,
  send    = 1,
  receive = 2,
};

class PatternCommunication {
public:
  int num = 0;
  simgrid::kernel::activity::CommImpl* comm_addr;
  PatternCommunicationType type = PatternCommunicationType::send;
  unsigned long src_proc        = 0;
  unsigned long dst_proc        = 0;
  const char* src_host          = nullptr;
  const char* dst_host          = nullptr;
  std::string rdv;
  std::vector<char> data;
  int tag   = 0;
  int index = 0;

  PatternCommunication() { std::memset(&comm_addr, 0, sizeof(comm_addr)); }

  PatternCommunication dup() const
  {
    simgrid::mc::PatternCommunication res;
    // num?
    res.comm_addr = this->comm_addr;
    res.type      = this->type;
    // src_proc?
    // dst_proc?
    res.dst_proc = this->dst_proc;
    res.dst_host = this->dst_host;
    res.rdv      = this->rdv;
    res.data     = this->data;
    // tag?
    res.index = this->index;
    return res;
  }
};

/* On every state, each process has an entry of the following type.
 * This represents both the process and its transition because
 *   a process cannot have more than one enabled transition at a given time.
 */
class ProcessState {
  /* Possible exploration status of a process transition in a state.
   * Either the checker did not consider the transition, or it was considered and to do, or considered and done.
   */
  enum class InterleavingType {
    /** This process transition is not considered by the checker (yet?) */
    disabled = 0,
    /** The checker algorithm decided that this process transitions should be done at some point */
    todo,
    /** The checker algorithm decided that this should be done, but it was done in the meanwhile */
    done,
  };

  /** Exploration control information */
  InterleavingType state = InterleavingType::disabled;

public:
  /** Number of times that the process was considered to be executed */
  // TODO, make this private
  unsigned int times_considered = 0;

  bool isDisabled() const { return this->state == InterleavingType::disabled; }
  bool isDone() const { return this->state == InterleavingType::done; }
  bool isTodo() const { return this->state == InterleavingType::todo; }
  /** Mark that we should try executing this process at some point in the future of the checker algorithm */
  void consider()
  {
    this->state            = InterleavingType::todo;
    this->times_considered = 0;
  }
  void setDone() { this->state = InterleavingType::done; }
};

/* A node in the exploration graph (kind-of)
 */
class XBT_PRIVATE State {
public:
  /** Sequential state number (used for debugging) */
  int num = 0;

  /** State's exploration status by process */
  std::vector<ProcessState> actorStates;

  Transition transition;

  /** The simcall which was executed, going out of that state */
  s_smx_simcall executed_req;

  /* Internal translation of the executed_req simcall
   *
   * SIMCALL_COMM_TESTANY is translated to a SIMCALL_COMM_TEST
   * and SIMCALL_COMM_WAITANY to a SIMCALL_COMM_WAIT.
   */
  s_smx_simcall internal_req;

  /* Can be used as a copy of the remote synchro object */
  simgrid::mc::Remote<simgrid::kernel::activity::CommImpl> internal_comm;

  /** Snapshot of system state (if needed) */
  std::shared_ptr<simgrid::mc::Snapshot> system_state;

  // For CommunicationDeterminismChecker
  std::vector<std::vector<simgrid::mc::PatternCommunication>> incomplete_comm_pattern;
  std::vector<unsigned> communicationIndices;

  explicit State(unsigned long state_number);

  std::size_t interleaveSize() const;
  void addInterleavingSet(smx_actor_t actor) { this->actorStates[actor->pid_].consider(); }
  Transition getTransition() const;
};
}
}

XBT_PRIVATE smx_simcall_t MC_state_get_request(simgrid::mc::State* state);

#endif
