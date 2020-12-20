/* config - Dictionary where the type of each cell is provided.             */
/* This is useful to build named structs, like option or property sets.     */

/* Copyright (c) 2004-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef XBT_CONFIG_H
#define XBT_CONFIG_H

#include <stdarg.h>
#include <xbt/base.h>

/** @addtogroup XBT_config
 *  @brief Changing the configuration of SimGrid components (grounding feature)
 *
 *  All modules of the SimGrid toolkit can be configured with this API.
 *  User modules and libraries can also use these facilities to handle their own configuration.
 *
 *  A configuration set contain several @e variables which have a unique name in the set and can take a given type of
 *  value. For example, it may contain a @a size variable, accepting @e int values.
 *
 *  It is impossible to set a value to a variable which has not been registered before.
 *  Usually, the module registers all the options it accepts in the configuration set, during its initialization and
 *  user code then set and unset values.
 *
 *  The easiest way to register a variable is to use the xbt_str_register_str function, which accepts a string
 *  representation of the config element descriptor. The syntax is the following:
 *  @verbatim <name>:<min nb>_to_<max nb>_<type>@endverbatim
 *
 *  For example, <tt>size:1_to_1_int</tt> describes a variable called @e size which must take exactly one value, and
 *  the value being an integer. Set the maximum to 0 to disable the upper bound on data count.
 *
 *  Another example could be <tt>outputfiles:0_to_10_string</tt> which describes a variable called @e outputfiles and
 *  which can take between 0 and 10 strings as value.
 *
 *  To some extend, configuration sets can be seen as typed hash structures.
 *
 *  @section XBT_cfg_ex Example of use
 *
 *  TBD
 */
/** @defgroup XBT_cfg_use User interface: changing values
 *  @ingroup XBT_config
 *
 * This is the only interface you should use unless you want to let your own code become configurable with this.
 *
 * If the variable accept at most one value, those functions replace the current value with the provided one. If max>1,
 * the provided value is appended to the list.
 *
 * string values are strdup'ed before use, so you can (and should) free your copy
 *
 * @{
 */
/** @brief Configuration set's data type is opaque. */
#ifdef __cplusplus
#include <xbt/config.hpp>
using xbt_cfg_t = simgrid::config::Config*;
#else
typedef void* xbt_cfg_t;
#endif

SG_BEGIN_DECL

/* Set the value of the cell @a name in @a cfg with the provided value.*/
XBT_PUBLIC void sg_cfg_set_int(const char* name, int val);
XBT_PUBLIC void sg_cfg_set_double(const char* name, double val);
XBT_PUBLIC void sg_cfg_set_boolean(const char* name, const char* val);
XBT_PUBLIC void sg_cfg_set_string(const char* name, const char* val);
/* @} */

/** @defgroup XBT_cfg_get Getting the stored values
 *  @ingroup XBT_config
 *
 * This is how to retrieve the values stored in the configuration set. This is only intended to configurable code,
 * naturally.
 *
 * Note that those function return a pointer to the values actually stored in the set. Do not modify them unless you
 * really know what you're doing. Likewise, do not free the strings after use, they are not copy of the data, but the
 * data themselves.
 *
 *  @{
 */

XBT_PUBLIC int sg_cfg_get_int(const char* name);
XBT_PUBLIC double sg_cfg_get_double(const char* name);
XBT_PUBLIC int sg_cfg_get_boolean(const char* name);
/** @} */

SG_END_DECL
#endif /* XBT_CONFIG_H */
