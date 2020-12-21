/* simgrid/config.h - Results of the configure made visible to user code.   */

/* Copyright (c) 2009-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_PUBLIC_CONFIG_H
#define SIMGRID_PUBLIC_CONFIG_H
#include <xbt/base.h>

/* Was MSG compiled in?  */
#define SIMGRID_HAVE_MSG 0
/* Was Jedule compiled in?  */
#define SIMGRID_HAVE_JEDULE 0
/* Was the Lua support compiled in? */
#define SIMGRID_HAVE_LUA 0
/* Were mallocators (object pools) compiled in? */
#define SIMGRID_HAVE_MALLOCATOR 1
/* Was the model-checking compiled in? */
#define SIMGRID_HAVE_MC 0
/* Was the ns-3 support compiled in? */
#define SIMGRID_HAVE_NS3 0
/* #undef NS3_MINOR_VERSION */
#endif /* SIMGRID_PUBLIC_CONFIG_H */
