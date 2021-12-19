/* Copyright (c) 2004-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef _SURF_SURF_PRIVATE_H
#define _SURF_SURF_PRIVATE_H

#include "surf/surf.h"
#include "surf/maxmin.h"
#include "surf/trace_mgr.h"
#include "xbt/log.h"
#include "surf/surfxml_parse.h"
#include "surf/random_mgr.h"
#include "instr/instr_private.h"
#include "surf/surfxml_parse_values.h"

#define NO_MAX_DURATION -1.0

SG_BEGIN_DECL()

extern xbt_dict_t watched_hosts_lib;

extern const char *surf_action_state_names[6];

/** @ingroup SURF_interface
 * @brief Possible update mechanisms
 */
typedef enum {
  UM_FULL,      /**< [TODO] */
  UM_LAZY,      /**< [TODO] */
  UM_UNDEFINED  /**< Mechanism not defined */
} e_UM_t;

/* Generic functions common to all models */

FILE *surf_fopen(const char *name, const char *mode);

extern tmgr_history_t history;

/* The __surf_is_absolute_file_path() returns 1 if
 * file_path is a absolute file path, in the other
 * case the function returns 0.
 */
int __surf_is_absolute_file_path(const char *file_path);

/**
 * Routing logic
 */

typedef struct s_model_type {
  const char *name;
  const char *desc;
  AS_t (*create) ();
  void (*end) (AS_t as);
} s_routing_model_description_t, *routing_model_description_t;

/* This enum used in the routing structure helps knowing in which situation we are. */
typedef enum {
  SURF_ROUTING_NULL = 0,   /**< Undefined type                                   */
  SURF_ROUTING_BASE,       /**< Base case: use simple link lists for routing     */
  SURF_ROUTING_RECURSIVE   /**< Recursive case: also return gateway informations */
} e_surf_routing_hierarchy_t;

XBT_PUBLIC(void) routing_model_create(void *loopback);
XBT_PUBLIC(void) routing_exit(void);
XBT_PUBLIC(void) storage_register_callbacks(void);

/* ***************************************** */
/* TUTORIAL: New TAG                         */
XBT_PUBLIC(void) gpu_register_callbacks(void);
/* ***************************************** */

XBT_PUBLIC(void) routing_register_callbacks(void);
XBT_PUBLIC(void) generic_free_route(sg_platf_route_cbarg_t route); // FIXME rename to routing_route_free
 // FIXME: make previous function private to routing again?


XBT_PUBLIC(void) routing_get_route_and_latency(sg_routing_edge_t src, sg_routing_edge_t dst,
                              xbt_dynar_t * route, double *latency);

XBT_PUBLIC(void) generic_get_graph(xbt_graph_t graph, xbt_dict_t nodes, xbt_dict_t edges, AS_t rc);
/**
 * Resource protected methods
 */
XBT_PUBLIC(void) surfxml_bufferstack_push(int _new);
XBT_PUBLIC(void) surfxml_bufferstack_pop(int _new);
XBT_PUBLIC(void) parse_after_config(void);

XBT_PUBLIC_DATA(int) surfxml_bufferstack_size;

/********** Tracing **********/
/* from surf_instr.c */
void TRACE_surf_host_set_power(double date, const char *resource, double power);
void TRACE_surf_link_set_bandwidth(double date, const char *resource, double bandwidth);

SG_END_DECL()

#endif                          /* _SURF_SURF_PRIVATE_H */
