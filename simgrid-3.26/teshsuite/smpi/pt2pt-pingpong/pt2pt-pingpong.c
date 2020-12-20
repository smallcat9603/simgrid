/* A simple example ping-pong program to test MPI_Send and MPI_Recv */

/* Copyright (c) 2009-2020. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <stdio.h>
#include <mpi.h>

static void test_opts(int argc, char* const argv[])
{
  int found = 0;
  int ret;
  while ((ret = getopt(argc, argv, "s")) >= 0) {
    if (ret == 's')
      found = 1;
  }
  if (found!=1){
    printf("(smpi_)getopt failed ! \n");
  }
}

int main(int argc, char *argv[])
{
  const int tag1 = 42;
  const int tag2 = 43; /* Message tag */
  int size;
  int rank;
  int msg = 99;
  MPI_Status status;
  int err = MPI_Init(&argc, &argv); /* Initialize MPI */

  /* test getopt function */
  test_opts(argc, argv);

  if (err != MPI_SUCCESS) {
    printf("MPI initialization failed!\n");
    exit(1);
  }
  MPI_Comm_size(MPI_COMM_WORLD, &size);   /* Get nr of tasks */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);   /* Get id of this process */
  if (size < 2) {
    printf("run this program with exactly 2 processes (-np 2)\n");
    MPI_Finalize();
    exit(0);
  }
  if (0 == rank) {
    printf("\n    *** Ping-pong test (MPI_Send/MPI_Recv) ***\n\n");
  }

  /* start ping-pong tests between several pairs */
  for (int pivot = 0; pivot < size - 1; pivot++) {
    if (pivot == rank) {
      printf("\n== pivot=%d : pingpong [%d] <--> [%d]\n", pivot, pivot, pivot + 1);

      int dst = rank + 1;
      printf("[%d] About to send 1st message '%d' to process [%d]\n", rank, msg, dst);
      MPI_Send(&msg, 1, MPI_INT, dst, tag1, MPI_COMM_WORLD);

      MPI_Recv(&msg, 1, MPI_INT, dst, tag2, MPI_COMM_WORLD, &status);     /* Receive a message */
      printf("[%d] Received reply message '%d' from process [%d]\n", rank, msg, dst);
    }
    if ((pivot + 1) == rank) {
      int src = rank - 1;
      MPI_Recv(&msg, 1, MPI_INT, src, tag1, MPI_COMM_WORLD, &status);     /* Receive a message */
      printf("[%d] Received 1st message '%d' from process [%d]\n", rank, msg, src);
      msg++;
      printf("[%d] increment message's value to  '%d'\n", rank, msg);
      printf("[%d] About to send back message '%d' to process [%d]\n", rank, msg, src);
      MPI_Send(&msg, 1, MPI_INT, src, tag2, MPI_COMM_WORLD);
    }
  }
  MPI_Finalize();
  return 0;
}
