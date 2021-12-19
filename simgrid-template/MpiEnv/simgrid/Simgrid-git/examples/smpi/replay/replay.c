/* Copyright (c) 2009-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <stdio.h>
#include "smpi/smpi.h"

int main(int argc, char *argv[])
{
  smpi_replay_init(&argc, &argv);

  /* Actually do the simulation using smpi_action_trace_run */
  smpi_action_trace_run(NULL);
  smpi_replay_finalize();

  return 0;
}
