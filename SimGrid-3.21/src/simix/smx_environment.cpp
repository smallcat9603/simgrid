/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "smx_private.hpp"
#include "src/include/surf/surf.hpp"
#include "xbt/xbt_os_time.h"

#include <simgrid/Exception.hpp>
#include <simgrid/engine.h>

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(simix_environment, simix, "Logging specific to SIMIX (environment)");

/********************************* SIMIX **************************************/

/**
 * @brief A platform constructor.
 *
 * Creates a new platform, including hosts, links and the
 * routing_table.
 * @param file a filename of a xml description of a platform. This file
 * follows this DTD :
 *
 *     @include surfxml.dtd
 *
 * Here is a small example of such a platform
 *
 *     @include small_platform.xml
 *
 */
void SIMIX_create_environment(std::string file)
{
  double start = 0;
  double end = 0;
  if(XBT_LOG_ISENABLED(simix_environment, xbt_log_priority_debug))
    start = xbt_os_time();
  try {
    parse_platform_file(file);
  }
  catch (xbt_ex& e) {
    xbt_die("Error while loading %s: %s", file.c_str(), e.what());
  }
  if(XBT_LOG_ISENABLED(simix_environment, xbt_log_priority_debug))
    end = xbt_os_time();
  XBT_DEBUG("PARSE TIME: %g", (end - start));
}

void SIMIX_create_environment(const char* file) // deprecated
{
  simgrid_load_platform(file);
}

void SIMIX_post_create_environment()
{
  surf_presolve();
}
