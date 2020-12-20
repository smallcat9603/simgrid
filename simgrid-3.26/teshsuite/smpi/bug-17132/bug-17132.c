/* Copyright (c) 2014-2020. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/* Bug report: https://gforge.inria.fr/tracker/index.php?func=detail&aid=17132&group_id=12&atid=165 */

#include <stdio.h>
#include <mpi.h>

int main(int argc, char ** argv)
{
  const int M = 8 * 1024;
  const int N = 32 * 1024;

  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  double *a = malloc(sizeof(double) * M);
  double *b = malloc(sizeof(double) * N);

  // A broadcast
  size_t err = MPI_Bcast(a, M, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  if (err != MPI_SUCCESS) {
    perror("Error Bcast A\n");
    MPI_Finalize();
    exit(-1);
  }

//  Uncommenting this barrier fixes it!
//  MPI_Barrier(MPI_COMM_WORLD);

  // Another broadcast
  err = MPI_Bcast(b, N, MPI_DOUBLE, 0, MPI_COMM_WORLD );
  if (err != MPI_SUCCESS) {
    perror("Error Bcast B\n");
    MPI_Finalize();
    exit(-1);
  }

  // Commenting out this barrier fixes it!!
  MPI_Barrier(MPI_COMM_WORLD);

  if(rank==0) {
    printf("Walltime = %g\n",MPI_Wtime());
  }

  MPI_Finalize();
  free(a);
  free(b);
  return 0;
}
