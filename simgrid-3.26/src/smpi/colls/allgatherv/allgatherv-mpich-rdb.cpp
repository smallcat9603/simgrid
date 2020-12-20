/* Copyright (c) 2013-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/* Short or medium size message and power-of-two no. of processes. Use
 * recursive doubling algorithm */

#include "../colls_private.hpp"
#include "smpi_status.hpp"
#include <algorithm>

namespace simgrid{
namespace smpi{

int allgatherv__mpich_rdb(
  const void *sendbuf,
  int sendcount,
  MPI_Datatype sendtype,
  void *recvbuf,
  const int *recvcounts,
  const int *displs,
  MPI_Datatype recvtype,
  MPI_Comm comm)
{
  unsigned int  j, i;
  MPI_Status status;
  MPI_Aint  recvtype_extent, recvtype_true_extent, recvtype_true_lb;
  unsigned int curr_cnt, dst, total_count;
  unsigned int mask, dst_tree_root, my_tree_root, position,
    send_offset, recv_offset, last_recv_cnt=0, nprocs_completed, k,
    offset, tmp_mask, tree_root;

  unsigned int comm_size = comm->size();
  unsigned int rank = comm->rank();

  total_count = 0;
  for (i=0; i<comm_size; i++)
    total_count += recvcounts[i];

  if (total_count == 0)
    return MPI_ERR_COUNT;

  recvtype_extent=recvtype->get_extent();

  /* need to receive contiguously into tmp_buf because
     displs could make the recvbuf noncontiguous */

  recvtype->extent(&recvtype_true_lb, &recvtype_true_extent);

  unsigned char* tmp_buf_rl = smpi_get_tmp_sendbuffer(total_count * std::max(recvtype_true_extent, recvtype_extent));

  /* adjust for potential negative lower bound in datatype */
  unsigned char* tmp_buf = tmp_buf_rl - recvtype_true_lb;

  /* copy local data into right location in tmp_buf */
  position = 0;
  for (i=0; i<rank; i++)
    position += recvcounts[i];
  if (sendbuf != MPI_IN_PLACE)
  {
    Datatype::copy(sendbuf, sendcount, sendtype, tmp_buf + position * recvtype_extent, recvcounts[rank], recvtype);
  }
  else
  {
    /* if in_place specified, local data is found in recvbuf */
    Datatype::copy(static_cast<char*>(recvbuf) + displs[rank] * recvtype_extent, recvcounts[rank], recvtype,
                   tmp_buf + position * recvtype_extent, recvcounts[rank], recvtype);
  }
  curr_cnt = recvcounts[rank];

  mask = 0x1;
  i = 0;
  while (mask < comm_size) {
    dst = rank ^ mask;

    /* find offset into send and recv buffers. zero out
       the least significant "i" bits of rank and dst to
       find root of src and dst subtrees. Use ranks of
       roots as index to send from and recv into buffer */

    dst_tree_root = dst >> i;
    dst_tree_root <<= i;

    my_tree_root = rank >> i;
    my_tree_root <<= i;

    if (dst < comm_size) {
      send_offset = 0;
      for (j=0; j<my_tree_root; j++)
        send_offset += recvcounts[j];

      recv_offset = 0;
      for (j=0; j<dst_tree_root; j++)
        recv_offset += recvcounts[j];

      Request::sendrecv(tmp_buf + send_offset * recvtype_extent, curr_cnt, recvtype, dst, COLL_TAG_ALLGATHERV,
                        tmp_buf + recv_offset * recvtype_extent, total_count - recv_offset, recvtype, dst,
                        COLL_TAG_ALLGATHERV, comm, &status);
      /* for convenience, recv is posted for a bigger amount
         than will be sent */
      last_recv_cnt=Status::get_count(&status, recvtype);
      curr_cnt += last_recv_cnt;
    }

    /* if some processes in this process's subtree in this step
       did not have any destination process to communicate with
       because of non-power-of-two, we need to send them the
       data that they would normally have received from those
       processes. That is, the haves in this subtree must send to
       the havenots. We use a logarithmic
       recursive-halfing algorithm for this. */

    /* This part of the code will not currently be
       executed because we are not using recursive
       doubling for non power of two. Mark it as experimental
       so that it doesn't show up as red in the coverage
       tests. */

    /* --BEGIN EXPERIMENTAL-- */
    if (dst_tree_root + mask > comm_size) {
      nprocs_completed = comm_size - my_tree_root - mask;
      /* nprocs_completed is the number of processes in this
         subtree that have all the data. Send data to others
         in a tree fashion. First find root of current tree
         that is being divided into two. k is the number of
         least-significant bits in this process's rank that
         must be zeroed out to find the rank of the root */
      j = mask;
      k = 0;
      while (j) {
        j >>= 1;
        k++;
      }
      k--;

      tmp_mask = mask >> 1;

      while (tmp_mask) {
        dst = rank ^ tmp_mask;

        tree_root = rank >> k;
        tree_root <<= k;

        /* send only if this proc has data and destination
           doesn't have data. at any step, multiple processes
           can send if they have the data */
        if ((dst > rank) &&
            (rank < tree_root + nprocs_completed)
            && (dst >= tree_root + nprocs_completed)) {

          offset = 0;
          for (j=0; j<(my_tree_root+mask); j++)
            offset += recvcounts[j];
          offset *= recvtype_extent;

          Request::send(tmp_buf + offset, last_recv_cnt, recvtype, dst, COLL_TAG_ALLGATHERV, comm);
          /* last_recv_cnt was set in the previous
             receive. that's the amount of data to be
             sent now. */
        }
        /* recv only if this proc. doesn't have data and sender
           has data */
        else if ((dst < rank) &&
                 (dst < tree_root + nprocs_completed) &&
                 (rank >= tree_root + nprocs_completed)) {

          offset = 0;
          for (j=0; j<(my_tree_root+mask); j++)
            offset += recvcounts[j];

          Request::recv(tmp_buf + offset * recvtype_extent, total_count - offset, recvtype, dst, COLL_TAG_ALLGATHERV,
                        comm, &status);
          /* for convenience, recv is posted for a
             bigger amount than will be sent */
          last_recv_cnt=Status::get_count(&status, recvtype);
          curr_cnt += last_recv_cnt;
        }
        tmp_mask >>= 1;
        k--;
      }
    }
    /* --END EXPERIMENTAL-- */

    mask <<= 1;
    i++;
  }

  /* copy data from tmp_buf to recvbuf */
  position = 0;
  for (j=0; j<comm_size; j++) {
    if ((sendbuf != MPI_IN_PLACE) || (j != rank)) {
      /* not necessary to copy if in_place and
         j==rank. otherwise copy. */
      Datatype::copy(tmp_buf + position * recvtype_extent, recvcounts[j], recvtype,
                     static_cast<char*>(recvbuf) + displs[j] * recvtype_extent, recvcounts[j], recvtype);
    }
    position += recvcounts[j];
  }

  smpi_free_tmp_buffer(tmp_buf_rl);
  return MPI_SUCCESS;
}

}
}
