/* Copyright (c) 2013-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_HOST_H_
#define SIMGRID_HOST_H_

#include <xbt/dict.h>
#include <xbt/dynar.h>
#include <simgrid/forward.h>

SG_BEGIN_DECL()
/** @brief Host datatype.
    @ingroup m_host_management

    A <em>location</em> (or <em>host</em>) is any possible place where an actor may run. Thus it is represented as a
    <em>physical resource with computing capabilities</em>, some <em>mailboxes</em> to enable running actors to
    communicate with remote ones, and some <em>private data</em> that can be only accessed by local actors.
 */

XBT_PUBLIC sg_host_t* sg_host_list();

/** @ingroup m_host_management
 * @brief Return the current number of hosts.
 */
XBT_PUBLIC size_t sg_host_count();

/** @ingroup m_host_management
 * @brief Return a dynar containing all the hosts declared at a given point of time (including VMs)
 * @remark The host order in the returned array is generally different from the host creation/declaration order in the
 *         XML platform (we use a hash table internally)
 */
XBT_PUBLIC xbt_dynar_t sg_hosts_as_dynar();

XBT_PUBLIC size_t sg_host_extension_create(void (*deleter)(void*));
XBT_PUBLIC void* sg_host_extension_get(sg_host_t host, size_t rank);

/** @ingroup m_host_management
 * @brief Finds a sg_host_t using its name.
 *
 * This is a name directory service
 * @param name the name of an host.
 * @return the corresponding host
 */
XBT_PUBLIC sg_host_t sg_host_by_name(const char* name);

/** @ingroup m_host_management
 *
 * @brief Return the name of the #sg_host_t. */
XBT_PUBLIC const char* sg_host_get_name(sg_host_t host);

// ========== User Data ==============
/** @ingroup m_host_management
 *
 * @brief Return the user data of a #sg_host_t.
 *
 * This functions returns the user data associated to @a host if it is possible.
 */
XBT_PUBLIC void* sg_host_user(sg_host_t host);
/** @ingroup m_host_management
 *
 * @brief Set the user data of a #sg_host_t.
 *
 * This functions attach @a data to @a host if it is possible.
 */
XBT_PUBLIC void sg_host_user_set(sg_host_t host, void* userdata);
XBT_PUBLIC void sg_host_user_destroy(sg_host_t host);

// ========= storage related functions ============
/** @ingroup m_host_management
 * @brief Return the list of mount point names on an host.
 * @param host a host
 * @return a dict containing all mount point on the host (mount_name => sg_storage_t)
 */
XBT_PUBLIC xbt_dict_t sg_host_get_mounted_storage_list(sg_host_t host);

/** @ingroup m_host_management
 * @brief Return the list of storages attached to an host.
 * @param host a host
 * @return a dynar containing all storages (name) attached to the host
 */
XBT_PUBLIC xbt_dynar_t sg_host_get_attached_storage_list(sg_host_t host);

// =========== user-level functions ===============
/** @ingroup m_host_management
 * @brief Return the speed of the processor (in flop/s), regardless of the current load on the machine.
 */
XBT_PUBLIC double sg_host_speed(sg_host_t host);
XBT_PUBLIC double sg_host_get_pstate_speed(sg_host_t host, int pstate_index);

XBT_PUBLIC double sg_host_get_available_speed(sg_host_t host);

XBT_PUBLIC int sg_host_core_count(sg_host_t host);

/** @ingroup m_process_management
 * @brief Return the location on which a process is running.
 * @return the sg_host_t corresponding to the location on which @a process is running.
 */
XBT_PUBLIC sg_host_t sg_host_self();

XBT_PUBLIC const char* sg_host_self_get_name();

/** @ingroup m_host_management
 * @brief Return the total count of pstates defined for a host. See also @ref plugin_energy.
 *
 * @param  host host to test
 */
XBT_PUBLIC int sg_host_get_nb_pstates(sg_host_t host);

XBT_PUBLIC int sg_host_get_pstate(sg_host_t host);
XBT_PUBLIC void sg_host_set_pstate(sg_host_t host, int pstate);

XBT_PUBLIC void sg_host_turn_on(sg_host_t host);
XBT_PUBLIC void sg_host_turn_off(sg_host_t host);
XBT_PUBLIC int sg_host_is_on(sg_host_t host);
XBT_PUBLIC int sg_host_is_off(sg_host_t host);

/** @ingroup m_host_management
 * @brief Returns a xbt_dict_t consisting of the list of properties assigned to this host
 *
 * @param host a host
 * @return a dict containing the properties
 */
XBT_PUBLIC xbt_dict_t sg_host_get_properties(sg_host_t host);

/** @ingroup m_host_management
 * @brief Returns the value of a given host property
 *
 * @param host a host
 * @param name a property name
 * @return value of a property (or nullptr if property not set)
 */
XBT_PUBLIC const char* sg_host_get_property_value(sg_host_t host, const char* name);

/** @ingroup m_host_management
 * @brief Change the value of a given host property
 *
 * @param host a host
 * @param name a property name
 * @param value what to change the property to
 */
XBT_PUBLIC void sg_host_set_property_value(sg_host_t host, const char* name, const char* value);

XBT_PUBLIC void sg_host_route(sg_host_t from, sg_host_t to, xbt_dynar_t links);
XBT_PUBLIC double sg_host_route_latency(sg_host_t from, sg_host_t to);
XBT_PUBLIC double sg_host_route_bandwidth(sg_host_t from, sg_host_t to);
XBT_PUBLIC void sg_host_dump(sg_host_t ws);

XBT_PUBLIC void sg_host_get_actor_list(sg_host_t host, xbt_dynar_t whereto);
SG_END_DECL()

#endif /* SIMGRID_HOST_H_ */
