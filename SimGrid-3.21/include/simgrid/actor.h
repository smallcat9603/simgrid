/* Copyright (c) 2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef INCLUDE_SIMGRID_ACTOR_H_
#define INCLUDE_SIMGRID_ACTOR_H_

#include <simgrid/forward.h>
#include <xbt/base.h>
#include <xbt/dict.h>

/* C interface */
SG_BEGIN_DECL()
/** @brief Actor datatype.
    @ingroup m_actor_management

    An actor may be defined as a <em>code</em>, with some <em>private data</em>, executing in a <em>location</em>.

    You should not access directly to the fields of the pointed structure, but always use the provided API to interact
    with actors.
 */
XBT_PUBLIC int sg_actor_get_PID(sg_actor_t actor);
XBT_PUBLIC int sg_actor_get_PPID(sg_actor_t actor);
XBT_PUBLIC sg_actor_t sg_actor_by_PID(aid_t pid);
XBT_PUBLIC const char* sg_actor_get_name(sg_actor_t actor);
XBT_PUBLIC sg_host_t sg_actor_get_host(sg_actor_t actor);
XBT_PUBLIC const char* sg_actor_get_property_value(sg_actor_t actor, const char* name);
XBT_PUBLIC xbt_dict_t sg_actor_get_properties(sg_actor_t actor);
XBT_PUBLIC void sg_actor_suspend(sg_actor_t actor);
XBT_PUBLIC void sg_actor_resume(sg_actor_t actor);
XBT_PUBLIC int sg_actor_is_suspended(sg_actor_t actor);
XBT_PUBLIC sg_actor_t sg_actor_restart(sg_actor_t actor);
void sg_actor_set_auto_restart(sg_actor_t actor, int auto_restart);
XBT_PUBLIC void sg_actor_daemonize(sg_actor_t actor);
XBT_PUBLIC void sg_actor_migrate(sg_actor_t process, sg_host_t host);
XBT_PUBLIC void sg_actor_join(sg_actor_t actor, double timeout);
XBT_PUBLIC void sg_actor_kill(sg_actor_t actor);
XBT_PUBLIC void sg_actor_kill_all();
XBT_PUBLIC void sg_actor_set_kill_time(sg_actor_t actor, double kill_time);
XBT_PUBLIC void sg_actor_yield();
SG_END_DECL()

#endif /* INCLUDE_SIMGRID_ACTOR_H_ */
