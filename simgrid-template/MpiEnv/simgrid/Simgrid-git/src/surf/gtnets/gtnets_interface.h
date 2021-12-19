/* Copyright (c) 2007-2010, 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

// Interface for GTNetS.
#ifndef _GTNETS_INTERFACE_H
#define _GTNETS_INTERFACE_H

#include "xbt/misc.h"           /* SG_{BEGIN,END}_DECL() */

SG_BEGIN_DECL()

int gtnets_initialize(int wsize);
int gtnets_add_link(int id, double bandwidth, double latency);
int gtnets_add_route(int src, int dst, int *links, int nlink);
int gtnets_add_router(int id);
int gtnets_add_onehop_route(int src, int dst, int link);
int gtnets_create_flow(int src, int dst, long datasize, void *metadata);
double gtnets_get_time_to_next_flow_completion();
double gtnets_run_until_next_flow_completion(void ***metadata,
                                             int *number_of_flows);
double gtnets_get_flow_rx(void *metadata);

void gtnets_print_topology(void);

int gtnets_run(double delta);
int gtnets_finalize();
void gtnets_set_jitter(double);
void gtnets_set_jitter_seed(int);

SG_END_DECL()

#endif
