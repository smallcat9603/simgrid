/* Copyright (c) 2009-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "src/smpi/include/smpi_actor.hpp"
#include "mc/mc.h"
#include "smpi_comm.hpp"
#include "src/mc/mc_replay.hpp"
#include "src/simix/smx_private.hpp"

#if HAVE_PAPI
#include "papi.h"
extern std::string papi_default_config_name;
#endif

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(smpi_process, smpi, "Logging specific to SMPI (kernel)");

namespace simgrid {
namespace smpi {

using simgrid::s4u::Actor;
using simgrid::s4u::ActorPtr;

ActorExt::ActorExt(ActorPtr actor, simgrid::s4u::Barrier* finalization_barrier)
    : finalization_barrier_(finalization_barrier), actor_(actor)
{
  mailbox_         = simgrid::s4u::Mailbox::by_name("SMPI-" + std::to_string(actor_->get_pid()));
  mailbox_small_   = simgrid::s4u::Mailbox::by_name("small-" + std::to_string(actor_->get_pid()));
  mailboxes_mutex_ = xbt_mutex_init();
  timer_           = xbt_os_timer_new();
  state_           = SmpiProcessState::UNINITIALIZED;
  if (MC_is_active())
    MC_ignore_heap(timer_, xbt_os_timer_size());

#if HAVE_PAPI
  if (not simgrid::config::get_value<std::string>("smpi/papi-events").empty()) {
    // TODO: Implement host/process/thread based counters. This implementation
    // just always takes the values passed via "default", like this:
    // "default:COUNTER1:COUNTER2:COUNTER3;".
    auto it = units2papi_setup.find(papi_default_config_name);
    if (it != units2papi_setup.end()) {
      papi_event_set_    = it->second.event_set;
      papi_counter_data_ = it->second.counter_data;
      XBT_DEBUG("Setting PAPI set for process %li", actor->get_pid());
    } else {
      papi_event_set_ = PAPI_NULL;
      XBT_DEBUG("No PAPI set for process %li", actor->get_pid());
    }
  }
#endif
}

ActorExt::~ActorExt()
{
  if (comm_self_ != MPI_COMM_NULL)
    simgrid::smpi::Comm::destroy(comm_self_);
  if (comm_intra_ != MPI_COMM_NULL)
    simgrid::smpi::Comm::destroy(comm_intra_);
  xbt_os_timer_free(timer_);
  xbt_mutex_destroy(mailboxes_mutex_);
}

void ActorExt::set_data(int* argc, char*** argv)
{
  instance_id_                   = std::string((*argv)[1]);
  comm_world_                    = smpi_deployment_comm_world(instance_id_);
  simgrid::s4u::Barrier* barrier = smpi_deployment_finalization_barrier(instance_id_);
  if (barrier != nullptr) // don't overwrite the current one if the instance has none
    finalization_barrier_ = barrier;

  if (*argc > 3) {
    memmove(&(*argv)[0], &(*argv)[2], sizeof(char*) * (*argc - 2));
    (*argv)[(*argc) - 1] = nullptr;
    (*argv)[(*argc) - 2] = nullptr;
  }
  (*argc) -= 2;
  // set the process attached to the mailbox
  mailbox_small_->set_receiver(actor_);
  XBT_DEBUG("<%ld> SMPI process has been initialized: %p", actor_->get_pid(), actor_.get());
}

/** @brief Prepares the current process for termination. */
void ActorExt::finalize()
{
  state_ = SmpiProcessState::FINALIZED;
  XBT_DEBUG("<%ld> Process left the game", actor_->get_pid());

  // This leads to an explosion of the search graph which cannot be reduced:
  if (MC_is_active() || MC_record_replay_is_active())
    return;
  // wait for all pending asynchronous comms to finish
  finalization_barrier_->wait();
}

/** @brief Check if a process is finalized */
int ActorExt::finalized()
{
  return (state_ == SmpiProcessState::FINALIZED);
}

/** @brief Check if a process is partially initialized already */
int ActorExt::initializing()
{
  return (state_ == SmpiProcessState::INITIALIZING);
}

/** @brief Check if a process is initialized */
int ActorExt::initialized()
{
  // TODO cheinrich: Check if we still need this. This should be a global condition, not for a
  // single process ... ?
  return (state_ == SmpiProcessState::INITIALIZED);
}

/** @brief Mark a process as initialized (=MPI_Init called) */
void ActorExt::mark_as_initialized()
{
  if (state_ != SmpiProcessState::FINALIZED)
    state_ = SmpiProcessState::INITIALIZED;
}

void ActorExt::set_replaying(bool value)
{
  if (state_ != SmpiProcessState::FINALIZED)
    replaying_ = value;
}

bool ActorExt::replaying()
{
  return replaying_;
}

ActorPtr ActorExt::get_actor()
{
  return actor_;
}

/**
 * @brief Returns a structure that stores the location (filename + linenumber) of the last calls to MPI_* functions.
 *
 * @see smpi_trace_set_call_location
 */
smpi_trace_call_location_t* ActorExt::call_location()
{
  return &trace_call_loc_;
}

void ActorExt::set_privatized_region(smpi_privatization_region_t region)
{
  privatized_region_ = region;
}

smpi_privatization_region_t ActorExt::privatized_region()
{
  return privatized_region_;
}

MPI_Comm ActorExt::comm_world()
{
  return comm_world_ == nullptr ? MPI_COMM_NULL : *comm_world_;
}

smx_mailbox_t ActorExt::mailbox()
{
  return mailbox_->get_impl();
}

smx_mailbox_t ActorExt::mailbox_small()
{
  return mailbox_small_->get_impl();
}

xbt_mutex_t ActorExt::mailboxes_mutex()
{
  return mailboxes_mutex_;
}

#if HAVE_PAPI
int ActorExt::papi_event_set()
{
  return papi_event_set_;
}

papi_counter_t& ActorExt::papi_counters()
{
  return papi_counter_data_;
}
#endif

xbt_os_timer_t ActorExt::timer()
{
  return timer_;
}

void ActorExt::simulated_start()
{
  simulated_ = SIMIX_get_clock();
}

double ActorExt::simulated_elapsed()
{
  return SIMIX_get_clock() - simulated_;
}

MPI_Comm ActorExt::comm_self()
{
  if (comm_self_ == MPI_COMM_NULL) {
    MPI_Group group = new Group(1);
    comm_self_      = new Comm(group, nullptr);
    group->set_mapping(actor_, 0);
  }
  return comm_self_;
}

MPI_Comm ActorExt::comm_intra()
{
  return comm_intra_;
}

void ActorExt::set_comm_intra(MPI_Comm comm)
{
  comm_intra_ = comm;
}

void ActorExt::set_sampling(int s)
{
  sampling_ = s;
}

int ActorExt::sampling()
{
  return sampling_;
}

void ActorExt::init(int* argc, char*** argv)
{

  if (smpi_process_count() == 0) {
    xbt_die("SimGrid was not initialized properly before entering MPI_Init. Aborting, please check compilation process "
            "and use smpirun\n");
  }
  if (argc != nullptr && argv != nullptr) {
    simgrid::s4u::ActorPtr proc = simgrid::s4u::Actor::self();
    proc->get_impl()->context_->set_cleanup(&SIMIX_process_cleanup);
    // cheinrich: I'm not sure what the impact of the SMPI_switch_data_segment on this call is. I moved
    // this up here so that I can set the privatized region before the switch.
    ActorExt* process = smpi_process_remote(proc);
    //if we are in MPI_Init and argc handling has already been done.
    if (process->initialized())
      return;
      
    process->state_ = SmpiProcessState::INITIALIZING;
    
    char* instance_id = (*argv)[1];
    try {
      int rank = std::stoi(std::string((*argv)[2]));
      smpi_deployment_register_process(instance_id, rank, proc);
    } catch (std::invalid_argument& ia) {
      throw std::invalid_argument(std::string("Invalid rank: ") + (*argv)[2]);
    }

    if (smpi_privatize_global_variables == SmpiPrivStrategies::MMAP) {
      /* Now using the segment index of this process  */
      process->set_privatized_region(smpi_init_global_memory_segment_process());
      /* Done at the process's creation */
      SMPI_switch_data_segment(proc);
    }

    process->set_data(argc, argv);
  } 
}

int ActorExt::get_optind()
{
  return optind;
}
void ActorExt::set_optind(int new_optind)
{
  optind = new_optind;
}

} // namespace smpi
} // namespace simgrid
