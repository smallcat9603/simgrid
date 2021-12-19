/* Copyright (c) 2009-2010, 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "mpi.h"

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif

//define MAXLEN  300000

int main(int argc, char *argv[])
{
  int rank, size;
  int i;
  int *sb;
  int *rb;
  int status;
  int mult=1;

  MPI_Init(&argc, &argv);
  int maxlen = argc >= 2 ? atoi(argv[1]) : 1;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  if (maxlen>1)mult=size;
  sb = (int *) xbt_malloc(size *maxlen * sizeof(int));
  rb = (int *) xbt_malloc(size *maxlen * sizeof(int));
  
  for (i = 0; i < size *maxlen; ++i) {
    sb[i] = rank*size + i;
    rb[i] = 0;
  }

  printf("[%d] sndbuf=[", rank);
  for (i = 0; i < size *mult; i++)
    printf("%d ", sb[i]);
  printf("]\n");

  status = MPI_Allreduce(sb, rb, size *maxlen, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  printf("[%d] rcvbuf=[", rank);
  for (i =  0; i < size *mult; i++)//do not print everything
    printf("%d ", rb[i]);
  printf("]\n");


  if (rank == 0) {
    if (status != MPI_SUCCESS) {
      printf("all_to_all returned %d\n", status);
      fflush(stdout);
    }
  }
  free(sb);
  free(rb);
  MPI_Finalize();
  return (EXIT_SUCCESS);
}
