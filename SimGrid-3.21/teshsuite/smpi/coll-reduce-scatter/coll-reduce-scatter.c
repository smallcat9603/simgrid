/* Copyright (c) 2013-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/*
 * Test of reduce scatter.
 * Each processor contributes its rank + the index to the reduction,  then receives the ith sum
 * Can be called with any number of processors.
 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char **argv )
{
    int err = 0;
    int toterr;
    int size;
    int rank;
    int i;
    MPI_Comm comm;

    MPI_Init( &argc, &argv );
    comm = MPI_COMM_WORLD;

    MPI_Comm_size( comm, &size );
    MPI_Comm_rank( comm, &rank );
    int* sendbuf = (int *) malloc( size * sizeof(int) );
    for (i=0; i<size; i++)
      sendbuf[i] = rank + i;
    int* recvcounts = (int*) malloc (size * sizeof(int));
    int* recvbuf    = (int*) malloc (size * sizeof(int));
    for (i=0; i<size; i++)
      recvcounts[i] = 1;
    MPI_Reduce_scatter( sendbuf, recvbuf, recvcounts, MPI_INT, MPI_SUM, comm );
    int sumval = size * rank + ((size - 1) * size)/2;
    /* recvbuf should be size * (rank + i) */
    if (recvbuf[0] != sumval) {
      err++;
      fprintf( stdout, "Did not get expected value for reduce scatter\n" );
      fprintf( stdout, "[%d] Got %d expected %d\n", rank, recvbuf[0], sumval );
    }

    MPI_Allreduce( &err, &toterr, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
    if (rank == 0 && toterr == 0) {
      printf( " No Errors\n" );
    }
    free(sendbuf);
    free(recvcounts);
    free(recvbuf);

    MPI_Finalize();

    return toterr;
}
