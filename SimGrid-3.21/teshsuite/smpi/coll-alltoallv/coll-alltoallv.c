/* Copyright (c) 2013-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

/* This program tests MPI_Alltoallv by having processor i send different amounts of data to each processor.

   TODO As there are separate send and receive types to alltoallv, there need to be tests to rearrange data on the fly.
   The first test sends i items to processor i from all processors.
   Currently, the test uses only MPI_INT; this is adequate for testing systems that use point-to-point operations

   Example values for 3 processes:
 * <0> sbuf: (#9):   [0][1][2][3][4][5][6][7][8]
   <0> scount: (#3): [0][1][2]
   <0> rcount: (#3): [0][0][0]
   <0> sdisp: (#3):  [0][1][3]
   <0> rdisp: (#3):  [0][0][0]

   <1> sbuf: (#9):   [100][101][102][103][104][105][106][107][108]
   <1> scount: (#3): [0][1][2]
   <1> rcount: (#3): [1][1][1]
   <1> sdisp: (#3):  [0][1][3]
   <1> rdisp: (#3):  [0][1][2]

   <2> sbuf: (#9):   [200][201][202][203][204][205][206][207][208]
   <2> scount: (#3): [0][1][2]
   <2> rcount: (#3): [2][2][2]
   <2> sdisp: (#3):  [0][1][3]
   <2> rdisp: (#3):  [0][2][4]

   after MPI_Alltoallv :
   <0> rbuf: (#9):   [-1][-1][-1][-1][-1][-1][-1][-1][-1]
   <1> rbuf: (#9):   [1][101][201][-1][-1][-1][-1][-1][-1]
   <2> rbuf: (#9):   [3][4][103][104][203][204][-1][-1][-1]
*/

static void print_buffer_int(void *buf, int len, const char *msg, int rank)
{
  printf("[%d] %s (#%d): ", rank, msg, len);
  for (int tmp = 0; tmp < len; tmp++) {
    int* v = buf;
    printf("[%d]", v[tmp]);
  }
  printf("\n");
}

int main(int argc, char **argv)
{
  MPI_Comm comm;
  int i;
  int rank;
  int size;

  MPI_Init(&argc, &argv);

  comm = MPI_COMM_WORLD;

  /* Create the buffer */
  MPI_Comm_size(comm, &size);
  if(size<=0){
    printf("error : comm size <= 0, run with mpirun\n");
    return -1;
  }
  MPI_Comm_rank(comm, &rank);
  int size2 = size * size;
  int* sbuf = (int*)xbt_malloc(size2 * sizeof(int));
  int* rbuf = (int*)xbt_malloc(size2 * sizeof(int));

  /* Load up the buffers */
  for (i = 0; i < size2; i++) {
    sbuf[i] = i + 100 * rank;
    rbuf[i] = -1;
  }

  /* Create and load the arguments to alltoallv */
  int* sendcounts = (int*)xbt_malloc(size * sizeof(int));
  int* recvcounts = (int*)xbt_malloc(size * sizeof(int));
  int* rdispls    = (int*)xbt_malloc(size * sizeof(int));
  int* sdispls    = (int*)xbt_malloc(size * sizeof(int));
  for (i = 0; i < size; i++) {
    sendcounts[i] = i;
    recvcounts[i] = rank;
    rdispls[i] = i * rank;
    sdispls[i] = (i * (i + 1)) / 2;
  }

  print_buffer_int(sbuf, size2, "sbuf:", rank);
  print_buffer_int(sendcounts, size, "scount:", rank);
  print_buffer_int(recvcounts, size, "rcount:", rank);
  print_buffer_int(sdispls, size, "sdisp:", rank);
  print_buffer_int(rdispls, size, "rdisp:", rank);

  MPI_Alltoallv(sbuf, sendcounts, sdispls, MPI_INT, rbuf, recvcounts, rdispls, MPI_INT, comm);

  print_buffer_int(rbuf, size2, "rbuf:", rank);

  MPI_Barrier(MPI_COMM_WORLD);
  if (0 == rank) {
    printf("Alltoallv TEST COMPLETE.\n");
  }
  free(sdispls);
  free(rdispls);
  free(recvcounts);
  free(sendcounts);
  free(rbuf);
  free(sbuf);

  MPI_Finalize();
  return 0;
}
