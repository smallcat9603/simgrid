/* Public interface to the Link datatype                                    */

/* Copyright (c) 2015-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef INCLUDE_SIMGRID_LINK_H_
#define INCLUDE_SIMGRID_LINK_H_

#include <simgrid/forward.h>
#include <xbt/base.h>

/* C interface */
SG_BEGIN_DECL()
XBT_PUBLIC const char* sg_link_name(sg_link_t link);
XBT_PUBLIC sg_link_t sg_link_by_name(const char* name);
XBT_PUBLIC int sg_link_is_shared(sg_link_t link);
XBT_PUBLIC double sg_link_bandwidth(sg_link_t link);
XBT_PUBLIC double sg_link_latency(sg_link_t link);
XBT_PUBLIC void* sg_link_data(sg_link_t link);
XBT_PUBLIC void sg_link_data_set(sg_link_t link, void* data);
XBT_PUBLIC int sg_link_count();
XBT_PUBLIC sg_link_t* sg_link_list();
SG_END_DECL()

#endif /* INCLUDE_SIMGRID_LINK_H_ */
