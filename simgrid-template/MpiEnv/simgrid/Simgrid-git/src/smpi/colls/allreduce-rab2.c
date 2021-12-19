/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "colls_private.h"
//#include <star-reduction.c>

// this requires that count >= NP
int smpi_coll_tuned_allreduce_rab2(void *sbuff, void *rbuff,
                                   int count, MPI_Datatype dtype,
                                   MPI_Op op, MPI_Comm comm)
{
  MPI_Aint s_extent;
  int i, rank, nprocs;
  int nbytes, send_size, s_offset, r_offset;
  void *recv, *send, *tmp;
  /*
     #ifdef MPICH2_REDUCTION
     MPI_User_function * uop = MPIR_Op_table[op % 16 - 1];
     #else
     MPI_User_function *uop;
     struct MPIR_OP *op_ptr;
     op_ptr = MPIR_ToPointer(op);
     uop  = op_ptr->op;
     #endif
   */
  rank = smpi_comm_rank(comm);
  nprocs = smpi_comm_size(comm);


  s_extent = smpi_datatype_get_extent(dtype);

  // uneven count
  if (count % nprocs) {
    if (count < nprocs)
      send_size = nprocs;
    else
      send_size = (count + nprocs) / nprocs;
    nbytes = send_size * s_extent;

    send = (void *) smpi_get_tmp_sendbuffer(s_extent * send_size * nprocs);
    recv = (void *) smpi_get_tmp_recvbuffer(s_extent * send_size * nprocs);
    tmp = (void *) smpi_get_tmp_sendbuffer(nbytes);

    memcpy(send, sbuff, s_extent * count);

    mpi_coll_alltoall_fun(send, send_size, dtype, recv, send_size, dtype, comm);

    memcpy(tmp, recv, nbytes);

    for (i = 1, s_offset = nbytes; i < nprocs; i++, s_offset = i * nbytes)
      smpi_op_apply(op, (char *) recv + s_offset, tmp, &send_size, &dtype);

    mpi_coll_allgather_fun(tmp, send_size, dtype, recv, send_size, dtype, comm);
    memcpy(rbuff, recv, count * s_extent);

    smpi_free_tmp_buffer(recv);
    smpi_free_tmp_buffer(tmp);
    smpi_free_tmp_buffer(send);
  } else {
    send = sbuff;
    send_size = count / nprocs;
    nbytes = send_size * s_extent;
    r_offset = rank * nbytes;

    recv = (void *) smpi_get_tmp_recvbuffer(s_extent * send_size * nprocs);

    mpi_coll_alltoall_fun(send, send_size, dtype, recv, send_size, dtype, comm);

    memcpy((char *) rbuff + r_offset, recv, nbytes);

    for (i = 1, s_offset = nbytes; i < nprocs; i++, s_offset = i * nbytes)
      smpi_op_apply(op, (char *) recv + s_offset, (char *) rbuff + r_offset,
                     &send_size, &dtype);

    mpi_coll_allgather_fun((char *) rbuff + r_offset, send_size, dtype, rbuff, send_size,
                  dtype, comm);
    smpi_free_tmp_buffer(recv);
  }

  return MPI_SUCCESS;
}
