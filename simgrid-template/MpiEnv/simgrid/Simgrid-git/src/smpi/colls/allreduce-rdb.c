/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "colls_private.h"
//#include <star-reduction.c>

int smpi_coll_tuned_allreduce_rdb(void *sbuff, void *rbuff, int count,
                                  MPI_Datatype dtype, MPI_Op op, MPI_Comm comm)
{
  int nprocs, rank, tag = COLL_TAG_ALLREDUCE;
  int mask, dst, pof2, newrank, rem, newdst;
  MPI_Aint extent, lb;
  MPI_Status status;
  void *tmp_buf = NULL;
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
  nprocs=smpi_comm_size(comm);
  rank=smpi_comm_rank(comm);

  smpi_datatype_extent(dtype, &lb, &extent);
  tmp_buf = (void *) smpi_get_tmp_sendbuffer(count * extent);

  smpi_mpi_sendrecv(sbuff, count, dtype, rank, 500,
               rbuff, count, dtype, rank, 500, comm, &status);

  // find nearest power-of-two less than or equal to comm_size
  pof2 = 1;
  while (pof2 <= nprocs)
    pof2 <<= 1;
  pof2 >>= 1;

  rem = nprocs - pof2;

  // In the non-power-of-two case, all even-numbered
  // processes of rank < 2*rem send their data to
  // (rank+1). These even-numbered processes no longer
  // participate in the algorithm until the very end. The
  // remaining processes form a nice power-of-two. 

  if (rank < 2 * rem) {
    // even       
    if (rank % 2 == 0) {

      smpi_mpi_send(rbuff, count, dtype, rank + 1, tag, comm);

      // temporarily set the rank to -1 so that this
      // process does not pariticipate in recursive
      // doubling
      newrank = -1;
    } else                      // odd
    {
      smpi_mpi_recv(tmp_buf, count, dtype, rank - 1, tag, comm, &status);
      // do the reduction on received data. since the
      // ordering is right, it doesn't matter whether
      // the operation is commutative or not.
      smpi_op_apply(op, tmp_buf, rbuff, &count, &dtype);

      // change the rank 
      newrank = rank / 2;
    }
  }

  else                          // rank >= 2 * rem 
    newrank = rank - rem;

  // If op is user-defined or count is less than pof2, use
  // recursive doubling algorithm. Otherwise do a reduce-scatter
  // followed by allgather. (If op is user-defined,
  // derived datatypes are allowed and the user could pass basic
  // datatypes on one process and derived on another as long as
  // the type maps are the same. Breaking up derived
  // datatypes to do the reduce-scatter is tricky, therefore
  // using recursive doubling in that case.) 

  if (newrank != -1) {
    mask = 0x1;
    while (mask < pof2) {
      newdst = newrank ^ mask;
      // find real rank of dest 
      dst = (newdst < rem) ? newdst * 2 + 1 : newdst + rem;

      // Send the most current data, which is in recvbuf. Recv
      // into tmp_buf 
      smpi_mpi_sendrecv(rbuff, count, dtype, dst, tag, tmp_buf, count, dtype,
                   dst, tag, comm, &status);

      // tmp_buf contains data received in this step.
      // recvbuf contains data accumulated so far 

      // op is commutative OR the order is already right
      // we assume it is commuttive op
      //      if (op -> op_commute  || (dst < rank))
      if ((dst < rank)) {
        smpi_op_apply(op, tmp_buf, rbuff, &count, &dtype);
      } else                    // op is noncommutative and the order is not right
      {
        smpi_op_apply(op, rbuff, tmp_buf, &count, &dtype);

        // copy result back into recvbuf
        smpi_mpi_sendrecv(tmp_buf, count, dtype, rank, tag, rbuff, count,
                     dtype, rank, tag, comm, &status);
      }
      mask <<= 1;
    }
  }
  // In the non-power-of-two case, all odd-numbered processes of
  // rank < 2 * rem send the result to (rank-1), the ranks who didn't
  // participate above.

  if (rank < 2 * rem) {
    if (rank % 2)               // odd 
      smpi_mpi_send(rbuff, count, dtype, rank - 1, tag, comm);
    else                        // even 
      smpi_mpi_recv(rbuff, count, dtype, rank + 1, tag, comm, &status);
  }

  smpi_free_tmp_buffer(tmp_buf);
  return MPI_SUCCESS;
}
