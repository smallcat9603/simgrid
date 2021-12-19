/* Copyright (c) 2010, 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "instr/instr_private.h"
#include "surf/surf_private.h"

#ifdef HAVE_TRACING

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(instr_surf, instr, "Tracing Surf");

void TRACE_surf_alloc(void)
{
  TRACE_surf_resource_utilization_alloc();
}

void TRACE_surf_release(void)
{
  TRACE_surf_resource_utilization_release();
}

void TRACE_surf_host_set_power(double date, const char *resource, double power)
{
  if (TRACE_categorized() || TRACE_uncategorized() || TRACE_platform()) {
    container_t container = PJ_container_get(resource);
    type_t type = PJ_type_get ("power", container->type);
    new_pajeSetVariable(date, container, type, power);
  }
}

void TRACE_surf_link_set_bandwidth(double date, const char *resource, double bandwidth)
{
  if (TRACE_categorized() || TRACE_uncategorized() || TRACE_platform()) {
    container_t container = PJ_container_get(resource);
    type_t type = PJ_type_get ("bandwidth", container->type);
    new_pajeSetVariable(date, container, type, bandwidth);
  }
}

/* to trace gtnets */
void TRACE_surf_gtnets_communicate(void *action, void *src, void *dst)
{
  /*FIXME:surf_action_network_GTNETS_t gtnets_action = (surf_action_network_GTNETS_t)action;
  gtnets_action->src = src;
  gtnets_action->dst = dst;*/
}

void TRACE_surf_action(surf_action_t surf_action, const char *category)
{
  if (!TRACE_is_enabled())
    return;
  if (!TRACE_categorized ())
    return;
  if (!category)
    return;

  surf_action_set_category(surf_action, category);
}
#endif /* HAVE_TRACING */
