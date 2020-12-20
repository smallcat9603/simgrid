/* Copyright (c) 2016-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <cassert>
#include <cstdio>

#include <memory>
#include <string>
#include <vector>

#include <xbt/log.h>
#include <xbt/sysdep.h>

#include "src/mc/Transition.hpp"
#include "src/mc/VisitedState.hpp"
#include "src/mc/checker/SafetyChecker.hpp"
#include "src/mc/mc_config.hpp"
#include "src/mc/mc_exit.hpp"
#include "src/mc/mc_private.hpp"
#include "src/mc/mc_record.hpp"
#include "src/mc/mc_request.hpp"
#include "src/mc/mc_smx.hpp"

#include "src/xbt/mmalloc/mmprivate.h"

using mcapi = simgrid::mc::mc_api;

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(mc_safety, mc, "Logging specific to MC safety verification ");

namespace simgrid {
namespace mc {

void SafetyChecker::check_non_termination(const State* current_state)
{
  for (auto state = stack_.rbegin(); state != stack_.rend(); ++state)
    if (mcapi::get().snapshot_equal((*state)->system_state_.get(), current_state->system_state_.get())) {
      XBT_INFO("Non-progressive cycle: state %d -> state %d", (*state)->num_, current_state->num_);
      XBT_INFO("******************************************");
      XBT_INFO("*** NON-PROGRESSIVE CYCLE DETECTED ***");
      XBT_INFO("******************************************");
      XBT_INFO("Counter-example execution trace:");
      auto checker = mcapi::get().mc_get_checker();
      for (auto const& s : checker->get_textual_trace())
        XBT_INFO("  %s", s.c_str());
      mcapi::get().dump_record_path();
      mcapi::get().log_state();

      throw TerminationError();
    }
}

RecordTrace SafetyChecker::get_record_trace() // override
{
  RecordTrace res;
  for (auto const& state : stack_)
    res.push_back(state->get_transition());
  return res;
}

std::vector<std::string> SafetyChecker::get_textual_trace() // override
{
  std::vector<std::string> trace;
  for (auto const& state : stack_) {
    int value         = state->transition_.argument_;
    smx_simcall_t req = &state->executed_req_;
    trace.push_back(mcapi::get().request_to_string(req, value, RequestType::executed));
  }
  return trace;
}

void SafetyChecker::log_state() // override
{
  XBT_INFO("Expanded states = %lu", expanded_states_count_);
  XBT_INFO("Visited states = %lu", mcapi::get().mc_get_visited_states());
  XBT_INFO("Executed transitions = %lu", mcapi::get().mc_get_executed_trans());
}

void SafetyChecker::run()
{
  /* This function runs the DFS algorithm the state space.
   * We do so iteratively instead of recursively, dealing with the call stack manually.
   * This allows one to explore the call stack at will. */

  while (not stack_.empty()) {
    /* Get current state */
    State* state = stack_.back().get();

    XBT_DEBUG("**************************************************");
    XBT_VERB("Exploration depth=%zu (state=%p, num %d)(%zu interleave)", stack_.size(), state, state->num_,
             state->interleave_size());

    mcapi::get().mc_inc_visited_states();

    // Backtrack if we reached the maximum depth
    if (stack_.size() > (std::size_t)_sg_mc_max_depth) {
      XBT_WARN("/!\\ Max depth reached ! /!\\ ");
      this->backtrack();
      continue;
    }

    // Backtrack if we are revisiting a state we saw previously
    if (visited_state_ != nullptr) {
      XBT_DEBUG("State already visited (equal to state %d), exploration stopped on this path.",
                visited_state_->original_num == -1 ? visited_state_->num : visited_state_->original_num);

      visited_state_ = nullptr;
      this->backtrack();
      continue;
    }

    // Search an enabled transition in the current state; backtrack if the interleave set is empty
    // get_request also sets state.transition to be the one corresponding to the returned req
    smx_simcall_t req = mcapi::get().mc_state_choose_request(state);
    // req is now the transition of the process that was selected to be executed

    if (req == nullptr) {
      XBT_DEBUG("There are no more processes to interleave. (depth %zu)", stack_.size() + 1);

      this->backtrack();
      continue;
    }

    // If there are processes to interleave and the maximum depth has not been
    // reached then perform one step of the exploration algorithm.
    XBT_DEBUG("Execute: %s", mcapi::get().request_to_string(req, state->transition_.argument_, RequestType::simix).c_str());

    std::string req_str;
    if (dot_output != nullptr)
      req_str = mcapi::get().request_get_dot_output(req, state->transition_.argument_);

    mcapi::get().mc_inc_executed_trans();

    /* Actually answer the request: let execute the selected request (MCed does one step) */
    mcapi::get().execute(state->transition_);

    /* Create the new expanded state (copy the state of MCed into our MCer data) */
    ++expanded_states_count_;
    auto next_state = std::make_unique<State>(expanded_states_count_);

    if (_sg_mc_termination)
      this->check_non_termination(next_state.get());

    /* Check whether we already explored next_state in the past (but only if interested in state-equality reduction) */
    if (_sg_mc_max_visited_states > 0)
      visited_state_ = visited_states_.addVisitedState(expanded_states_count_, next_state.get(), true);

    /* If this is a new state (or if we don't care about state-equality reduction) */
    if (visited_state_ == nullptr) {
      /* Get an enabled process and insert it in the interleave set of the next state */
      auto actors = mcapi::get().get_actors(); 
      for (auto& remoteActor : actors) {
        auto actor = remoteActor.copy.get_buffer();
        if (mcapi::get().actor_is_enabled(actor->get_pid())) {
          next_state->add_interleaving_set(actor);
          if (reductionMode_ == ReductionMode::dpor)
            break; // With DPOR, we take the first enabled transition
        }
      }

      if (dot_output != nullptr)
        std::fprintf(dot_output, "\"%d\" -> \"%d\" [%s];\n", state->num_, next_state->num_, req_str.c_str());

    } else if (dot_output != nullptr)
      std::fprintf(dot_output, "\"%d\" -> \"%d\" [%s];\n", state->num_,
                   visited_state_->original_num == -1 ? visited_state_->num : visited_state_->original_num,
                   req_str.c_str());

    stack_.push_back(std::move(next_state));
  }

  XBT_INFO("No property violation found.");
  mcapi::get().log_state();
}

void SafetyChecker::backtrack()
{
  stack_.pop_back();

  /* Check for deadlocks */
  if (mcapi::get().mc_check_deadlock()) {
    mcapi::get().mc_show_deadlock();
    throw DeadlockError();
  }

  /* Traverse the stack backwards until a state with a non empty interleave set is found, deleting all the states that
   *  have it empty in the way. For each deleted state, check if the request that has generated it (from its
   *  predecessor state), depends on any other previous request executed before it. If it does then add it to the
   *  interleave set of the state that executed that previous request. */

  while (not stack_.empty()) {
    std::unique_ptr<State> state = std::move(stack_.back());
    stack_.pop_back();
    if (reductionMode_ == ReductionMode::dpor) {
      smx_simcall_t req = &state->internal_req_;
      if (req->call_ == simix::Simcall::MUTEX_LOCK || req->call_ == simix::Simcall::MUTEX_TRYLOCK)
        xbt_die("Mutex is currently not supported with DPOR,  use --cfg=model-check/reduction:none");

      const kernel::actor::ActorImpl* issuer = mcapi::get().simcall_get_issuer(req);
      for (auto i = stack_.rbegin(); i != stack_.rend(); ++i) {
        State* prev_state = i->get();
        if (mcapi::get().request_depend(req, &prev_state->internal_req_)) {
          if (XBT_LOG_ISENABLED(mc_safety, xbt_log_priority_debug)) {
            XBT_DEBUG("Dependent Transitions:");
            int value              = prev_state->transition_.argument_;
            smx_simcall_t prev_req = &prev_state->executed_req_;
            XBT_DEBUG("%s (state=%d)", mcapi::get().request_to_string(prev_req, value, RequestType::internal).c_str(),
                      prev_state->num_);
            value    = state->transition_.argument_;
            prev_req = &state->executed_req_;
            XBT_DEBUG("%s (state=%d)", mcapi::get().request_to_string(prev_req, value, RequestType::executed).c_str(),
                      state->num_);
          }

          if (not prev_state->actor_states_[issuer->get_pid()].is_done())
            prev_state->add_interleaving_set(issuer);
          else
            XBT_DEBUG("Process %p is in done set", req->issuer_);
          break;
        } else if (req->issuer_ == prev_state->internal_req_.issuer_) {
          XBT_DEBUG("Simcall %s and %s with same issuer", mcapi::get().simcall_get_name(req->call_),
                    mcapi::get().simcall_get_name(prev_state->internal_req_.call_));
          break;
        } else {
          const kernel::actor::ActorImpl* previous_issuer = mcapi::get().simcall_get_issuer(&prev_state->internal_req_);
          XBT_DEBUG("Simcall %s, process %ld (state %d) and simcall %s, process %ld (state %d) are independent",
                    mcapi::get().simcall_get_name(req->call_), issuer->get_pid(), state->num_,
                    mcapi::get().simcall_get_name(prev_state->internal_req_.call_), previous_issuer->get_pid(), prev_state->num_);
        }
      }
    }

    if (state->interleave_size() && stack_.size() < (std::size_t)_sg_mc_max_depth) {
      /* We found a back-tracking point, let's loop */
      XBT_DEBUG("Back-tracking to state %d at depth %zu", state->num_, stack_.size() + 1);
      stack_.push_back(std::move(state));
      this->restore_state();
      XBT_DEBUG("Back-tracking to state %d at depth %zu done", stack_.back()->num_, stack_.size());
      break;
    } else {
      XBT_DEBUG("Delete state %d at depth %zu", state->num_, stack_.size() + 1);
    }
  }
}

void SafetyChecker::restore_state()
{
  /* Intermediate backtracking */
  const State* last_state = stack_.back().get();
  if (last_state->system_state_) {
    mc_api::get().restore_state(last_state->system_state_);
    return;
  }

  /* Restore the initial state */
  mcapi::get().restore_initial_state();

  /* Traverse the stack from the state at position start and re-execute the transitions */
  for (std::unique_ptr<State> const& state : stack_) {
    if (state == stack_.back())
      break;
    mcapi::get().execute(state->transition_);
    /* Update statistics */
    mcapi::get().mc_inc_visited_states();
    mcapi::get().mc_inc_executed_trans();
  }
}

SafetyChecker::SafetyChecker() : Checker()
{
  reductionMode_ = reduction_mode;
  if (_sg_mc_termination)
    reductionMode_ = ReductionMode::none;
  else if (reductionMode_ == ReductionMode::unset)
    reductionMode_ = ReductionMode::dpor;

  if (_sg_mc_termination)
    XBT_INFO("Check non progressive cycles");
  else
    XBT_INFO("Check a safety property. Reduction is: %s.",
             (reductionMode_ == ReductionMode::none ? "none"
                                                    : (reductionMode_ == ReductionMode::dpor ? "dpor" : "unknown")));
  
  mcapi::get().session_initialize();  

  XBT_DEBUG("Starting the safety algorithm");

  ++expanded_states_count_;
  auto initial_state = std::make_unique<State>(expanded_states_count_);

  XBT_DEBUG("**************************************************");
  XBT_DEBUG("Initial state");

  /* Get an enabled actor and insert it in the interleave set of the initial state */
  auto actors = mcapi::get().get_actors();
  for (auto& actor : actors)
    if (mcapi::get().actor_is_enabled(actor.copy.get_buffer()->get_pid())) {
      initial_state->add_interleaving_set(actor.copy.get_buffer());
      if (reductionMode_ != ReductionMode::none)
        break;
    }

  stack_.push_back(std::move(initial_state));
}

Checker* createSafetyChecker()
{
  return new SafetyChecker();
}

} // namespace mc
} // namespace simgrid
