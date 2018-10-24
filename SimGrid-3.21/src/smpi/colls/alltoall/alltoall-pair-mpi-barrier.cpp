/* Copyright (c) 2013-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "../colls_private.hpp"
/*****************************************************************************

 * Function: alltoall_pair_mpi_barrier

 * Return: int

 * Inputs:
    send_buff: send input buffer
    send_count: number of elements to send
    send_type: data type of elements being sent
    recv_buff: receive output buffer
    recv_count: number of elements to received
    recv_type: data type of elements being received
    comm: communicator

 * Descrp: Function works when P is power of two. In each phase of P - 1
           phases, nodes in pair communicate their data. MPI barriers are
           inserted between each two phases.

 * Auther: Ahmad Faraj

 ****************************************************************************/
namespace simgrid{
namespace smpi{
int
Coll_alltoall_pair_mpi_barrier::alltoall(void *send_buff, int send_count,
                                          MPI_Datatype send_type,
                                          void *recv_buff, int recv_count,
                                          MPI_Datatype recv_type, MPI_Comm comm)
{
  MPI_Status s;
  MPI_Aint send_chunk, recv_chunk;
  int i, src, dst, rank, num_procs;
  int tag = COLL_TAG_ALLTOALL;
  char *send_ptr = (char *) send_buff;
  char *recv_ptr = (char *) recv_buff;

  rank = comm->rank();
  num_procs = comm->size();

  if((num_procs&(num_procs-1)))
    THROWF(arg_error,0, "alltoall pair algorithm can't be used with non power of two number of processes ! ");

  send_chunk = send_type->get_extent();
  recv_chunk = recv_type->get_extent();

  send_chunk *= send_count;
  recv_chunk *= recv_count;

  for (i = 0; i < num_procs; i++) {
    src = dst = rank ^ i;
    Colls::barrier(comm);
    Request::sendrecv(send_ptr + dst * send_chunk, send_count, send_type, dst,
                 tag, recv_ptr + src * recv_chunk, recv_count, recv_type,
                 src, tag, comm, &s);
  }
  return MPI_SUCCESS;
}
}
}
