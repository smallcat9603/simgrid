/* Copyright (c) 2007-2020. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "chainsend.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(chainsend, "Messages specific for chainsend");

int main(int argc, char* argv[])
{
  simgrid_init(&argc, argv);

  simgrid_load_platform(argv[1]);

  /* Trace categories */
  TRACE_category_with_color("host0", "0 0 1");
  TRACE_category_with_color("host1", "0 1 0");
  TRACE_category_with_color("host2", "0 1 1");
  TRACE_category_with_color("host3", "1 0 0");
  TRACE_category_with_color("host4", "1 0 1");
  TRACE_category_with_color("host5", "0 0 0");
  TRACE_category_with_color("host6", "1 1 0");
  TRACE_category_with_color("host7", "1 1 1");
  TRACE_category_with_color("host8", "0 1 0");

  /*   Application deployment */
  simgrid_register_function("broadcaster", broadcaster);
  simgrid_register_function("peer", peer);

  simgrid_load_deployment(argv[2]);

  simgrid_run();
  XBT_INFO("Total simulation time: %e", simgrid_get_clock());

  return 0;
}
