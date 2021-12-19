/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "colls_private.h"

int smpi_coll_tuned_allgather_loosely_lr(void *sbuf, int scount,
                                         MPI_Datatype stype, void *rbuf,
                                         int rcount, MPI_Datatype rtype,
                                         MPI_Comm comm)
{
  int comm_size, rank;
  int tag = COLL_TAG_ALLGATHER;
  int i, j, send_offset, recv_offset;
  int intra_rank, inter_rank, inter_comm_size, intra_comm_size;
  int inter_dst, inter_src;

  comm_size = smpi_comm_size(comm);

if(smpi_comm_get_leaders_comm(comm)==MPI_COMM_NULL){
    smpi_comm_init_smp(comm);
  }
  int num_core=1;
  if (smpi_comm_is_uniform(comm)){
    num_core = smpi_comm_size(smpi_comm_get_intra_comm(comm));
  }

  if(comm_size%num_core)
    THROWF(arg_error,0, "allgather loosely lr algorithm can't be used with non multiple of NUM_CORE=%d number of processes ! ",num_core);

  rank = smpi_comm_rank(comm);
  MPI_Aint rextent, sextent;
  rextent = smpi_datatype_get_extent(rtype);
  sextent = smpi_datatype_get_extent(stype);
  MPI_Request inter_rrequest;
  MPI_Request rrequest_array[128];
  MPI_Request srequest_array[128];
  MPI_Request inter_srequest_array[128];


  int rrequest_count = 0;
  int srequest_count = 0;
  int inter_srequest_count = 0;

  MPI_Status status;

  intra_rank = rank % num_core;
  inter_rank = rank / num_core;
  inter_comm_size = (comm_size + num_core - 1) / num_core;
  intra_comm_size = num_core;

  int src_seg, dst_seg;

  //copy corresponding message from sbuf to rbuf
  recv_offset = rank * rextent * rcount;
  smpi_mpi_sendrecv(sbuf, scount, stype, rank, tag,
               (char *)rbuf + recv_offset, rcount, rtype, rank, tag, comm, &status);

  int dst, src;
  int inter_send_offset, inter_recv_offset;

  rrequest_count = 0;
  srequest_count = 0;
  inter_srequest_count = 0;

  for (i = 0; i < inter_comm_size; i++) {

    // inter_communication

    inter_dst = (rank + intra_comm_size) % comm_size;
    inter_src = (rank - intra_comm_size + comm_size) % comm_size;

    src_seg =
        ((inter_rank - 1 - i +
          inter_comm_size) % inter_comm_size) * intra_comm_size + intra_rank;
    dst_seg =
        ((inter_rank - i +
          inter_comm_size) % inter_comm_size) * intra_comm_size + intra_rank;

    inter_send_offset = dst_seg * sextent * scount;
    inter_recv_offset = src_seg * rextent * rcount;

    for (j = 0; j < intra_comm_size; j++) {

      // inter communication
      if (intra_rank == j) {
        if (i != inter_comm_size - 1) {

          inter_rrequest = smpi_mpi_irecv((char *)rbuf + inter_recv_offset, rcount, rtype,
	                                  inter_src, tag, comm);
          inter_srequest_array[inter_srequest_count++] = smpi_mpi_isend((char *)rbuf + inter_send_offset, scount, stype,
			                                                inter_dst, tag, comm);
        }
      }
      //intra_communication
      src = inter_rank * intra_comm_size + j;
      dst = inter_rank * intra_comm_size + j;

      src_seg =
          ((inter_rank - i +
            inter_comm_size) % inter_comm_size) * intra_comm_size + j;
      dst_seg =
          ((inter_rank - i +
            inter_comm_size) % inter_comm_size) * intra_comm_size + intra_rank;

      send_offset = dst_seg * sextent * scount;
      recv_offset = src_seg * rextent * rcount;


      if (j != intra_rank) {

        rrequest_array[rrequest_count++] = smpi_mpi_irecv((char *)rbuf + recv_offset, rcount, rtype, src, tag, comm);
        srequest_array[srequest_count++] = smpi_mpi_isend((char *)rbuf + send_offset, scount, stype, dst, tag, comm);

      }
    }                           // intra loop


    // wait for inter communication to finish for these rounds (# of round equals num_core)
    if (i != inter_comm_size - 1) {
      smpi_mpi_wait(&inter_rrequest, &status);
    }

  }                             //inter loop

  smpi_mpi_waitall(rrequest_count, rrequest_array, MPI_STATUSES_IGNORE);
  smpi_mpi_waitall(srequest_count, srequest_array, MPI_STATUSES_IGNORE);
  smpi_mpi_waitall(inter_srequest_count, inter_srequest_array, MPI_STATUSES_IGNORE);

  return MPI_SUCCESS;
}
