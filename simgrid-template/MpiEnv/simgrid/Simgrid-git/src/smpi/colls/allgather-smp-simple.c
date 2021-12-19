/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "colls_private.h"

int smpi_coll_tuned_allgather_smp_simple(void *send_buf, int scount,
                                         MPI_Datatype stype, void *recv_buf,
                                         int rcount, MPI_Datatype rtype,
                                         MPI_Comm comm)
{
  int src, dst, comm_size, rank;
  comm_size = smpi_comm_size(comm);

  if(smpi_comm_get_leaders_comm(comm)==MPI_COMM_NULL){
    smpi_comm_init_smp(comm);
  }
  int num_core=1;
  if (smpi_comm_is_uniform(comm)){
    num_core = smpi_comm_size(smpi_comm_get_intra_comm(comm));
  }

  if(comm_size%num_core)
     THROWF(arg_error,0, "allgather SMP simple algorithm can't be used with non multiple of NUM_CORE=%d number of processes ! ", num_core);

  rank = smpi_comm_rank(comm);
  MPI_Aint rextent, sextent;
  rextent = smpi_datatype_get_extent(rtype);
  sextent = smpi_datatype_get_extent(stype);
  int tag = COLL_TAG_ALLGATHER;
  MPI_Status status;
  int i, send_offset, recv_offset;
  int intra_rank, inter_rank;
  intra_rank = rank % num_core;
  inter_rank = rank / num_core;
  int inter_comm_size = (comm_size + num_core - 1) / num_core;
  int num_core_in_current_smp = num_core;

  // the last SMP node may have fewer number of running processes than all others
  if (inter_rank == (inter_comm_size - 1)) {
    num_core_in_current_smp = comm_size - (inter_rank * num_core);
  }
  //INTRA-SMP-ALLGATHER
  recv_offset = rank * rextent * rcount;
  smpi_mpi_sendrecv(send_buf, scount, stype, rank, tag,
               ((char *) recv_buf + recv_offset), rcount, rtype, rank, tag,
               comm, &status);
  for (i = 1; i < num_core_in_current_smp; i++) {

    dst =
        (inter_rank * num_core) + (intra_rank + i) % (num_core_in_current_smp);
    src =
        (inter_rank * num_core) + (intra_rank - i +
                                   num_core_in_current_smp) %
        (num_core_in_current_smp);
    recv_offset = src * rextent * rcount;

    smpi_mpi_sendrecv(send_buf, scount, stype, dst, tag,
                 ((char *) recv_buf + recv_offset), rcount, rtype, src, tag,
                 comm, &status);

  }

  // INTER-SMP-ALLGATHER 
  // Every root of each SMP node post INTER-Sendrecv, then do INTRA-Bcast for each receiving message



  if (intra_rank == 0) {
    MPI_Request *reqs, *req_ptr;
    int num_req = (inter_comm_size - 1) * 2;
    reqs = (MPI_Request *) xbt_malloc(num_req * sizeof(MPI_Request));
    req_ptr = reqs;
    MPI_Status *stat;
    stat = (MPI_Status *) xbt_malloc(num_req * sizeof(MPI_Status));

    for (i = 1; i < inter_comm_size; i++) {

      //dst = ((inter_rank+i)%inter_comm_size) * num_core;
      src = ((inter_rank - i + inter_comm_size) % inter_comm_size) * num_core;
      //send_offset = (rank * sextent * scount);
      recv_offset = (src * sextent * scount);
      //      smpi_mpi_sendrecv((recv_buf+send_offset), (scount * num_core), stype, dst, tag, 
      //             (recv_buf+recv_offset), (rcount * num_core), rtype, src, tag, comm, &status);
      //MPIC_Isend((recv_buf+send_offset), (scount * num_core), stype, dst, tag, comm, req_ptr++);
      *(req_ptr++) = smpi_mpi_irecv(((char *) recv_buf + recv_offset), (rcount * num_core), rtype,
                src, tag, comm);
    }
    for (i = 1; i < inter_comm_size; i++) {

      dst = ((inter_rank + i) % inter_comm_size) * num_core;
      //src = ((inter_rank-i+inter_comm_size)%inter_comm_size) * num_core;
      send_offset = (rank * sextent * scount);
      //recv_offset = (src * sextent * scount);
      //      smpi_mpi_sendrecv((recv_buf+send_offset), (scount * num_core), stype, dst, tag, 
      //             (recv_buf+recv_offset), (rcount * num_core), rtype, src, tag, comm, &status);
      *(req_ptr++) = smpi_mpi_isend(((char *) recv_buf + send_offset), (scount * num_core), stype,
                dst, tag, comm);
      //MPIC_Irecv((recv_buf+recv_offset), (rcount * num_core), rtype, src, tag, comm, req_ptr++);
    }
    smpi_mpi_waitall(num_req, reqs, stat);
    free(reqs);
    free(stat);

  }
  //INTRA-BCAST (use flat tree)

  if (intra_rank == 0) {
    for (i = 1; i < num_core_in_current_smp; i++) {
      //printf("rank = %d, num = %d send to %d\n",rank, num_core_in_current_smp, (rank + i));
      smpi_mpi_send(recv_buf, (scount * comm_size), stype, (rank + i), tag, comm);
    }
  } else {
    //printf("rank = %d recv from %d\n",rank, (inter_rank * num_core));
    smpi_mpi_recv(recv_buf, (rcount * comm_size), rtype, (inter_rank * num_core),
             tag, comm, &status);
  }


  return MPI_SUCCESS;
}
