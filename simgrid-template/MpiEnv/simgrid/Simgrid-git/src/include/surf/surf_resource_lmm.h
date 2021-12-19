
/* Copyright (c) 2009-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SURF_RESOURCE_LMM_H
#define SURF_RESOURCE_LMM_H
#include "surf/surf.h"
#include "surf/trace_mgr.h"
#include "surf/surf_resource.h"

#ifdef TOMATO
static XBT_INLINE
    surf_resource_lmm_t surf_resource_lmm_new(size_t childsize,
                                              /* for superclass */
                                              surf_model_t model,
                                              const char *name, xbt_dict_t props,
                                              lmm_system_t system,
                                              double constraint_value,
                                              tmgr_history_t history,
                                              e_surf_resource_state_t state_init,
                                              tmgr_trace_t state_trace,
                                              double metric_peak,
                                              tmgr_trace_t metric_trace)
{

  surf_resource_lmm_t res =
      (surf_resource_lmm_t) surf_resource_new(childsize, model, name,
                                              props, NULL);

  res->constraint = lmm_constraint_new(system, res, constraint_value);
  res->state_current = state_init;
  if (state_trace)
    res->state_event =
        tmgr_history_add_trace(history, state_trace, 0.0, 0, res);

  res->power.scale = 1.0;
  res->power.peak = metric_peak;
  if (metric_trace)
    res->power.event =
        tmgr_history_add_trace(history, metric_trace, 0.0, 0, res);
  return res;
}
#endif

static XBT_INLINE e_surf_resource_state_t surf_resource_lmm_get_state(void
                                                                      *r)
{
  surf_resource_lmm_t resource = (surf_resource_lmm_t) r;
  return (resource)->state_current;
}
#endif                          /* SURF_RESOURCE_LMM_H */
