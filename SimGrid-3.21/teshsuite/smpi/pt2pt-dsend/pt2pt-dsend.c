/* Copyright (c) 2011-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/* This program simply does a very small exchange to test whether using SIMIX dsend to model the eager mode works */
#include <stdint.h>
#include <stdio.h>
#include <mpi.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(dsend,"the dsend test");

int main()
{
  int rank;
  int32_t data=11;

  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Request r;
  if (rank==1) {
    data=22;
    MPI_Send(&data,1,MPI_INT32_T,(rank+1)%2,666,MPI_COMM_WORLD);
  } else {
    MPI_Recv(&data,1,MPI_INT32_T,MPI_ANY_SOURCE,666,MPI_COMM_WORLD,NULL);
    if (data !=22) {
      printf("rank %d: Damn, data does not match (got %d)\n",rank, data);
    }
  }

  if (rank==1) {
    data=22;
    MPI_Isend(&data,1,MPI_INT32_T,(rank+1)%2,666,MPI_COMM_WORLD, &r);
    MPI_Wait(&r, MPI_STATUS_IGNORE);
  } else {
    MPI_Irecv(&data,1,MPI_INT32_T,MPI_ANY_SOURCE,666,MPI_COMM_WORLD,&r);
    MPI_Wait(&r, MPI_STATUS_IGNORE);
    if (data !=22) {
      printf("rank %d: Damn, data does not match (got %d)\n",rank, data);
    }
  }

  XBT_INFO("rank %d: data exchanged", rank);
  MPI_Finalize();
  return 0;
}
