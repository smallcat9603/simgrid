/* virtu - virtualization layer for the logging to know about the actors    */

/* Copyright (c) 2007-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef XBT_VIRTU_H
#define XBT_VIRTU_H

#include <xbt/base.h>

#ifdef __cplusplus
#include <string>
#include <vector>

namespace simgrid {
namespace xbt {

/* Get the name of the UNIX process englobing the world */
XBT_PUBLIC_DATA std::string binary_name;
/** Contains all the parameters we got from the command line (including argv[0]) */
XBT_PUBLIC_DATA std::vector<std::string> cmdline;

} // namespace xbt
} // namespace simgrid
#endif

SG_BEGIN_DECL

XBT_PUBLIC const char* xbt_procname(void);

XBT_PUBLIC int xbt_getpid(void);

SG_END_DECL

#endif /* XBT_VIRTU_H */
