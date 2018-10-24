/* Copyright (c) 2009-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
  int rank;
  int size;
  int i;
  int status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int* sb = (int *) xbt_malloc(size * sizeof(int) * 2);
  int* rb = (int *) xbt_malloc(size * sizeof(int) * 2);

  for (i = 0; i < size; ++i) {
    sb[i] = rank*size + i;
    rb[i] = 0;
  }

  printf("[%d] sndbuf=[", rank);
  for (i = 0; i < size; i++)
    printf("%d ", sb[i]);
  printf("]\n");

  status = MPI_Alltoall(sb, 1, MPI_INT, rb, 1, MPI_INT, MPI_COMM_WORLD);

  printf("[%d] rcvbuf=[", rank);
  for (i = 0; i < size; i++)
    printf("%d ", rb[i]);
  printf("]\n");

  if (rank == 0 && status != MPI_SUCCESS) {
    printf("all_to_all returned %d\n", status);
    fflush(stdout);
  }
  xbt_free(sb);
  xbt_free(rb);
  MPI_Finalize();
  return (EXIT_SUCCESS);
}
