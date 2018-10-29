/* Copyright (c) 2009-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt/replay.hpp"
#include "smpi/smpi.h"

int main(int argc, char* argv[])
{
  /* Setup things and register default actions */
  smpi_replay_init(&argc, &argv);

  /* The regular run of the replayer */
  smpi_replay_main(&argc, &argv);
  return 0;
}
