/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "colls_private.h"
//#include <star-reduction.c>

int
smpi_coll_tuned_reduce_flat_tree(void *sbuf, void *rbuf, int count,
                                 MPI_Datatype dtype, MPI_Op op,
                                 int root, MPI_Comm comm)
{
  int i, tag = COLL_TAG_REDUCE;
  int size;
  int rank;
  MPI_Aint extent;
  char *origin = 0;
  char *inbuf;
  MPI_Status status;

  rank = smpi_comm_rank(comm);
  size = smpi_comm_size(comm);

  /* If not root, send data to the root. */
  extent = smpi_datatype_get_extent(dtype);

  if (rank != root) {
    smpi_mpi_send(sbuf, count, dtype, root, tag, comm);
    return 0;
  }

  /* Root receives and reduces messages.  Allocate buffer to receive
     messages. */

  if (size > 1)
    origin = (char *) smpi_get_tmp_recvbuffer(count * extent);


  /* Initialize the receive buffer. */
  if (rank == (size - 1))
    smpi_mpi_sendrecv(sbuf, count, dtype, rank, tag,
                 rbuf, count, dtype, rank, tag, comm, &status);
  else
    smpi_mpi_recv(rbuf, count, dtype, size - 1, tag, comm, &status);

  /* Loop receiving and calling reduction function (C or Fortran). */

  for (i = size - 2; i >= 0; --i) {
    if (rank == i)
      inbuf = sbuf;
    else {
      smpi_mpi_recv(origin, count, dtype, i, tag, comm, &status);
      inbuf = origin;
    }

    /* Call reduction function. */
    smpi_op_apply(op, inbuf, rbuf, &count, &dtype);

  }

  if (origin)
    smpi_free_tmp_buffer(origin);

  /* All done */
  return 0;
}
