/* Copyright (c) 2018-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef INCLUDE_SIMGRID_EXEC_H_
#define INCLUDE_SIMGRID_EXEC_H_

#include <simgrid/forward.h>
#include <xbt/dynar.h>

/* C interface */
SG_BEGIN_DECL

XBT_PUBLIC void sg_exec_set_bound(sg_exec_t exec, double bound);
XBT_PUBLIC const char* sg_exec_get_name(const_sg_exec_t exec);
XBT_PUBLIC void sg_exec_set_name(sg_exec_t exec, const char* name);
XBT_PUBLIC void sg_exec_set_host(sg_exec_t exec, sg_host_t new_host);
XBT_PUBLIC double sg_exec_get_remaining(const_sg_exec_t exec);
XBT_PUBLIC double sg_exec_get_remaining_ratio(const_sg_exec_t exec);

XBT_PUBLIC void sg_exec_start(sg_exec_t exec);
XBT_PUBLIC void sg_exec_cancel(sg_exec_t exec);
XBT_PUBLIC int sg_exec_test(sg_exec_t exec);
XBT_PUBLIC sg_error_t sg_exec_wait(sg_exec_t exec);
XBT_PUBLIC sg_error_t sg_exec_wait_for(sg_exec_t exec, double timeout);
XBT_PUBLIC int sg_exec_wait_any_for(sg_exec_t* execs, size_t count, double timeout);
XBT_PUBLIC int sg_exec_wait_any(sg_exec_t* execs, size_t count);

SG_END_DECL

#endif /* INCLUDE_SIMGRID_EXEC_H_ */
