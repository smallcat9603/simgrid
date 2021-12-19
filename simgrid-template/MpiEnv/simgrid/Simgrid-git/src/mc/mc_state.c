/* Copyright (c) 2008-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "../simix/smx_private.h"
#include "xbt/fifo.h"
#include "mc_private.h"

/**
 * \brief Creates a state data structure used by the exploration algorithm
 */
mc_state_t MC_state_new()
{
  mc_state_t state = NULL;

  state = xbt_new0(s_mc_state_t, 1);
  state->max_pid = simix_process_maxpid;
  state->proc_status = xbt_new0(s_mc_procstate_t, state->max_pid);
  state->system_state = NULL;
  state->num = ++mc_stats->expanded_states;

  return state;
}

/**
 * \brief Deletes a state data structure
 * \param trans The state to be deleted
 */
void MC_state_delete(mc_state_t state)
{
  if (state->system_state)
    MC_free_snapshot(state->system_state);
  xbt_free(state->proc_status);
  xbt_free(state);
}

void MC_state_interleave_process(mc_state_t state, smx_process_t process)
{
  state->proc_status[process->pid].state = MC_INTERLEAVE;
  state->proc_status[process->pid].interleave_count = 0;
}

void MC_state_remove_interleave_process(mc_state_t state, smx_process_t process)
{
  if (state->proc_status[process->pid].state == MC_INTERLEAVE)
    state->proc_status[process->pid].state = MC_DONE;
}

unsigned int MC_state_interleave_size(mc_state_t state)
{
  unsigned int i, size = 0;

  for (i = 0; i < state->max_pid; i++) {
    if ((state->proc_status[i].state == MC_INTERLEAVE)
        || (state->proc_status[i].state == MC_MORE_INTERLEAVE))
      size++;
  }

  return size;
}

int MC_state_process_is_done(mc_state_t state, smx_process_t process)
{
  return state->proc_status[process->pid].state == MC_DONE ? TRUE : FALSE;
}

void MC_state_set_executed_request(mc_state_t state, smx_simcall_t req,
                                   int value)
{
  state->executed_req = *req;
  state->req_num = value;
  smx_process_t process = NULL;
  mc_procstate_t procstate = NULL;

  /* The waitany and testany request are transformed into a wait or test request over the
   * corresponding communication action so it can be treated later by the dependence
   * function. */
  switch (req->call) {
  case SIMCALL_COMM_WAITANY:
    state->internal_req.call = SIMCALL_COMM_WAIT;
    state->internal_req.issuer = req->issuer;
    state->internal_comm =
        *xbt_dynar_get_as(simcall_comm_waitany__get__comms(req), value,
                          smx_synchro_t);
    simcall_comm_wait__set__comm(&state->internal_req, &state->internal_comm);
    simcall_comm_wait__set__timeout(&state->internal_req, 0);
    break;

  case SIMCALL_COMM_TESTANY:
    state->internal_req.call = SIMCALL_COMM_TEST;
    state->internal_req.issuer = req->issuer;

    if (value > 0)
      state->internal_comm =
          *xbt_dynar_get_as(simcall_comm_testany__get__comms(req), value,
                            smx_synchro_t);

    simcall_comm_test__set__comm(&state->internal_req, &state->internal_comm);
    simcall_comm_test__set__result(&state->internal_req, value);
    break;

  case SIMCALL_COMM_WAIT:
    state->internal_req = *req;
    state->internal_comm = *(simcall_comm_wait__get__comm(req));
    simcall_comm_wait__set__comm(&state->executed_req, &state->internal_comm);
    simcall_comm_wait__set__comm(&state->internal_req, &state->internal_comm);
    break;

  case SIMCALL_COMM_TEST:
    state->internal_req = *req;
    state->internal_comm = *simcall_comm_test__get__comm(req);
    simcall_comm_test__set__comm(&state->executed_req, &state->internal_comm);
    simcall_comm_test__set__comm(&state->internal_req, &state->internal_comm);
    break;

  case SIMCALL_MC_RANDOM:
    state->internal_req = *req;
    if (value != simcall_mc_random__get__max(req)) {
      xbt_swag_foreach(process, simix_global->process_list) {
        procstate = &state->proc_status[process->pid];
        if (process->pid == req->issuer->pid) {
          procstate->state = MC_MORE_INTERLEAVE;
          break;
        }
      }
    }
    break;

  default:
    state->internal_req = *req;
    break;
  }
}

smx_simcall_t MC_state_get_executed_request(mc_state_t state, int *value)
{
  *value = state->req_num;
  return &state->executed_req;
}

smx_simcall_t MC_state_get_internal_request(mc_state_t state)
{
  return &state->internal_req;
}

smx_simcall_t MC_state_get_request(mc_state_t state, int *value)
{
  smx_process_t process = NULL;
  mc_procstate_t procstate = NULL;
  unsigned int start_count;
  smx_synchro_t act = NULL;

  xbt_swag_foreach(process, simix_global->process_list) {
    procstate = &state->proc_status[process->pid];

    if (procstate->state == MC_INTERLEAVE
        || procstate->state == MC_MORE_INTERLEAVE) {
      if (MC_process_is_enabled(process)) {
        switch (process->simcall.call) {
        case SIMCALL_COMM_WAITANY:
          *value = -1;
          while (procstate->interleave_count <
                 xbt_dynar_length(simcall_comm_waitany__get__comms
                                  (&process->simcall))) {
            if (MC_request_is_enabled_by_idx
                (&process->simcall, procstate->interleave_count++)) {
              *value = procstate->interleave_count - 1;
              break;
            }
          }

          if (procstate->interleave_count >=
              xbt_dynar_length(simcall_comm_waitany__get__comms
                               (&process->simcall)))
            procstate->state = MC_DONE;

          if (*value != -1)
            return &process->simcall;

          break;

        case SIMCALL_COMM_TESTANY:
          start_count = procstate->interleave_count;
          *value = -1;
          while (procstate->interleave_count <
                 xbt_dynar_length(simcall_comm_testany__get__comms
                                  (&process->simcall))) {
            if (MC_request_is_enabled_by_idx
                (&process->simcall, procstate->interleave_count++)) {
              *value = procstate->interleave_count - 1;
              break;
            }
          }

          if (procstate->interleave_count >=
              xbt_dynar_length(simcall_comm_testany__get__comms
                               (&process->simcall)))
            procstate->state = MC_DONE;

          if (*value != -1 || start_count == 0)
            return &process->simcall;

          break;

        case SIMCALL_COMM_WAIT:
          act = simcall_comm_wait__get__comm(&process->simcall);
          if (act->comm.src_proc && act->comm.dst_proc) {
            *value = 0;
          } else {
            if (act->comm.src_proc == NULL && act->comm.type == SIMIX_COMM_READY
                && act->comm.detached == 1)
              *value = 0;
            else
              *value = -1;
          }
          procstate->state = MC_DONE;
          return &process->simcall;

          break;

        case SIMCALL_MC_RANDOM:
          if (procstate->state == MC_INTERLEAVE)
            *value = 0;
          else {
            if (state->req_num < simcall_mc_random__get__max(&process->simcall))
              *value = state->req_num + 1;
          }
          procstate->state = MC_DONE;
          return &process->simcall;
          break;

        default:
          procstate->state = MC_DONE;
          *value = 0;
          return &process->simcall;
          break;
        }
      }
    }
  }

  return NULL;
}
