/* src/simgrid/version.h - internal versioning info                        */

/* Copyright (c) 2009-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_VERSION_H
#define SIMGRID_VERSION_H

#define SIMGRID_GIT_VERSION   "0e789e05d5"

/** Define the version numbers of the used header files.
  See sg_version_get() to retrieve the version of the dynamic library. */
#define SIMGRID_VERSION_MAJOR 3
#define SIMGRID_VERSION_MINOR 26
#define SIMGRID_VERSION_PATCH 0
#define SIMGRID_INSTALL_PREFIX "/opt/simgrid3.26"

SG_BEGIN_DECL
/** Retrieves the version numbers of the used dynamic library (so, DLL or dynlib), while
    SIMGRID_VERSION_MAJOR and friends give the version numbers of the used header files */
XBT_PUBLIC void sg_version_get(int* major, int* minor, int* patch);

/** Display the version information and some additional blurb. */
XBT_PUBLIC void sg_version();
SG_END_DECL


/* Version as a single integer. v3.4 is 30400, v3.16.2 is 31602, v42 will be 420000, and so on. */
#define SIMGRID_VERSION (100UL * (100UL * (SIMGRID_VERSION_MAJOR) + (SIMGRID_VERSION_MINOR)) + (SIMGRID_VERSION_PATCH))

#define SIMGRID_VERSION_STRING "SimGrid version 3.26"

#endif /* SIMGRID_VERSION_H */
