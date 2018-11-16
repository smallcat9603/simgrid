/* Copyright (c) 2009-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <simgrid/s4u/Actor.hpp>
#include "smpi/smpi.h"
#include "smpi/sampi.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(smpi_plugin_load_balancer_example, smpi, "Simple tracing test for SAMPI functions");

int main(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);
  void* pointer = malloc(100 * sizeof(int));
  free(pointer);
  pointer = malloc(100 * sizeof(int));
  int rank;
  int err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);   /* Get id of this process */
  if (err != MPI_SUCCESS) {
    fprintf(stderr, "MPI_Comm_rank failed: %d", err);
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    exit(EXIT_FAILURE);
  }
  AMPI_Iteration_in(MPI_COMM_WORLD);
  simgrid::s4u::this_actor::sleep_for(rank);
  AMPI_Iteration_out(MPI_COMM_WORLD);

  AMPI_Iteration_in(MPI_COMM_WORLD);
  simgrid::s4u::this_actor::sleep_for(rank);
  AMPI_Iteration_out(MPI_COMM_WORLD);
  if (rank == 0)
    free(pointer);
  AMPI_Migrate(MPI_COMM_WORLD);
  if (rank != 0)
  free(pointer);

  MPI_Finalize();
  return 0;
}

