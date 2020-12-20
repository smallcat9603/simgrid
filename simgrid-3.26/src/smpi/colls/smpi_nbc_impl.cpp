/* Asynchronous parts of the basic collective algorithms, meant to be used both for the naive default implementation, but also for non blocking collectives */

/* Copyright (c) 2009-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "colls_private.hpp"
#include "src/smpi/include/smpi_actor.hpp"

namespace simgrid{
namespace smpi{

int colls::ibarrier(MPI_Comm comm, MPI_Request* request, int external)
{
  int size = comm->size();
  int rank = comm->rank();
  int system_tag=COLL_TAG_BARRIER-external;
  (*request) = new Request( nullptr, 0, MPI_BYTE,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT);
  if (rank > 0) {
    auto* requests = new MPI_Request[2];
    requests[0] = Request::isend (nullptr, 0, MPI_BYTE, 0,
                             system_tag,
                             comm);
    requests[1] = Request::irecv (nullptr, 0, MPI_BYTE, 0,
                             system_tag,
                             comm);
    (*request)->set_nbc_requests(requests, 2);
  }
  else {
    auto* requests = new MPI_Request[(size - 1) * 2];
    for (int i = 1; i < 2 * size - 1; i += 2) {
      requests[i - 1] = Request::irecv(nullptr, 0, MPI_BYTE, MPI_ANY_SOURCE, system_tag, comm);
      requests[i]     = Request::isend(nullptr, 0, MPI_BYTE, (i + 1) / 2, system_tag, comm);
    }
    (*request)->set_nbc_requests(requests, 2*(size-1));
  }
  return MPI_SUCCESS;
}

int colls::ibcast(void* buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm, MPI_Request* request,
                  int external)
{
  int size = comm->size();
  int rank = comm->rank();
  int system_tag=COLL_TAG_BCAST-external;
  (*request) = new Request( nullptr, 0, MPI_BYTE,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT);
  if (rank != root) {
    auto* requests = new MPI_Request[1];
    requests[0] = Request::irecv (buf, count, datatype, root,
                             system_tag,
                             comm);
    (*request)->set_nbc_requests(requests, 1);
  }
  else {
    auto* requests = new MPI_Request[size - 1];
    int n = 0;
    for (int i = 0; i < size; i++) {
      if(i!=root){
        requests[n] = Request::isend(buf, count, datatype, i,
                                 system_tag,
                                 comm
                                 );
        n++;
      }
    }
    (*request)->set_nbc_requests(requests, size-1);
  }
  return MPI_SUCCESS;
}

int colls::iallgather(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, int recvcount,
                      MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request, int external)
{

  const int system_tag = COLL_TAG_ALLGATHER-external;
  MPI_Aint lb = 0;
  MPI_Aint recvext = 0;

  int rank = comm->rank();
  int size = comm->size();
  (*request) = new Request( nullptr, 0, MPI_BYTE,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT);
  // FIXME: check for errors
  recvtype->extent(&lb, &recvext);
  // Local copy from self
  Datatype::copy(sendbuf, sendcount, sendtype, static_cast<char *>(recvbuf) + rank * recvcount * recvext, recvcount,
                     recvtype);
  // Send/Recv buffers to/from others;
  auto* requests = new MPI_Request[2 * (size - 1)];
  int index = 0;
  for (int other = 0; other < size; other++) {
    if(other != rank) {
      requests[index] = Request::isend_init(sendbuf, sendcount, sendtype, other, system_tag,comm);
      index++;
      requests[index] = Request::irecv_init(static_cast<char *>(recvbuf) + other * recvcount * recvext, recvcount, recvtype,
                                        other, system_tag, comm);
      index++;
    }
  }
  Request::startall(2 * (size - 1), requests);
  (*request)->set_nbc_requests(requests, 2 * (size - 1));
  return MPI_SUCCESS;
}

int colls::iscatter(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, int recvcount,
                    MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request, int external)
{
  const int system_tag = COLL_TAG_SCATTER-external;
  MPI_Aint lb = 0;
  MPI_Aint sendext = 0;

  int rank = comm->rank();
  int size = comm->size();
  (*request) = new Request( nullptr, 0, MPI_BYTE,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT);
  if(rank != root) {
    auto* requests = new MPI_Request[1];
    // Recv buffer from root
    requests[0] = Request::irecv(recvbuf, recvcount, recvtype, root, system_tag, comm);
    (*request)->set_nbc_requests(requests, 1);
  } else {
    sendtype->extent(&lb, &sendext);
    // Local copy from root
    if(recvbuf!=MPI_IN_PLACE){
        Datatype::copy(static_cast<const char *>(sendbuf) + root * sendcount * sendext,
                           sendcount, sendtype, recvbuf, recvcount, recvtype);
    }
    // Send buffers to receivers
    auto* requests = new MPI_Request[size - 1];
    int index = 0;
    for(int dst = 0; dst < size; dst++) {
      if(dst != root) {
        requests[index] = Request::isend_init(static_cast<const char *>(sendbuf) + dst * sendcount * sendext, sendcount, sendtype,
                                          dst, system_tag, comm);
        index++;
      }
    }
    // Wait for completion of isend's.
    Request::startall(size - 1, requests);
    (*request)->set_nbc_requests(requests, size - 1);
  }
  return MPI_SUCCESS;
}

int colls::iallgatherv(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, const int* recvcounts,
                       const int* displs, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request, int external)
{
  const int system_tag = COLL_TAG_ALLGATHERV-external;
  MPI_Aint lb = 0;
  MPI_Aint recvext = 0;

  int rank = comm->rank();
  int size = comm->size();
  (*request) = new Request( nullptr, 0, MPI_BYTE,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT);
  recvtype->extent(&lb, &recvext);
  // Local copy from self
  Datatype::copy(sendbuf, sendcount, sendtype,
                     static_cast<char *>(recvbuf) + displs[rank] * recvext,recvcounts[rank], recvtype);
  // Send buffers to others;
  auto* requests = new MPI_Request[2 * (size - 1)];
  int index = 0;
  for (int other = 0; other < size; other++) {
    if(other != rank) {
      requests[index] =
        Request::isend_init(sendbuf, sendcount, sendtype, other, system_tag, comm);
      index++;
      requests[index] = Request::irecv_init(static_cast<char *>(recvbuf) + displs[other] * recvext, recvcounts[other],
                          recvtype, other, system_tag, comm);
      index++;
    }
  }
  // Wait for completion of all comms.
  Request::startall(2 * (size - 1), requests);
  (*request)->set_nbc_requests(requests, 2 * (size - 1));
  return MPI_SUCCESS;
}

int colls::ialltoall(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, int recvcount,
                     MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request, int external)
{
  int system_tag   = COLL_TAG_ALLTOALL-external;
  MPI_Aint lb      = 0;
  MPI_Aint sendext = 0;
  MPI_Aint recvext = 0;

  /* Initialize. */
  int rank = comm->rank();
  int size = comm->size();
  (*request) = new Request( nullptr, 0, MPI_BYTE,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT);
  sendtype->extent(&lb, &sendext);
  recvtype->extent(&lb, &recvext);
  /* simple optimization */
  int err = Datatype::copy(static_cast<const char *>(sendbuf) + rank * sendcount * sendext, sendcount, sendtype,
                               static_cast<char *>(recvbuf) + rank * recvcount * recvext, recvcount, recvtype);
  if (err == MPI_SUCCESS && size > 1) {
    /* Initiate all send/recv to/from others. */
    auto* requests = new MPI_Request[2 * (size - 1)];
    /* Post all receives first -- a simple optimization */
    int count = 0;
    for (int i = (rank + 1) % size; i != rank; i = (i + 1) % size) {
      requests[count] = Request::irecv_init(static_cast<char *>(recvbuf) + i * recvcount * recvext, recvcount,
                                        recvtype, i, system_tag, comm);
      count++;
    }
    /* Now post all sends in reverse order
     *   - We would like to minimize the search time through message queue
     *     when messages actually arrive in the order in which they were posted.
     * TODO: check the previous assertion
     */
    for (int i = (rank + size - 1) % size; i != rank; i = (i + size - 1) % size) {
      requests[count] = Request::isend_init(static_cast<const char *>(sendbuf) + i * sendcount * sendext, sendcount,
                                        sendtype, i, system_tag, comm);
      count++;
    }
    /* Wait for them all. */
    Request::startall(count, requests);
    (*request)->set_nbc_requests(requests, count);
  }
  return MPI_SUCCESS;
}

int colls::ialltoallv(const void* sendbuf, const int* sendcounts, const int* senddisps, MPI_Datatype sendtype,
                      void* recvbuf, const int* recvcounts, const int* recvdisps, MPI_Datatype recvtype, MPI_Comm comm,
                      MPI_Request* request, int external)
{
  const int system_tag = COLL_TAG_ALLTOALLV-external;
  MPI_Aint lb = 0;
  MPI_Aint sendext = 0;
  MPI_Aint recvext = 0;

  /* Initialize. */
  int rank = comm->rank();
  int size = comm->size();
  (*request) = new Request( nullptr, 0, MPI_BYTE,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT);
  sendtype->extent(&lb, &sendext);
  recvtype->extent(&lb, &recvext);
  /* Local copy from self */
  int err = Datatype::copy(static_cast<const char *>(sendbuf) + senddisps[rank] * sendext, sendcounts[rank], sendtype,
                               static_cast<char *>(recvbuf) + recvdisps[rank] * recvext, recvcounts[rank], recvtype);
  if (err == MPI_SUCCESS && size > 1) {
    /* Initiate all send/recv to/from others. */
    auto* requests = new MPI_Request[2 * (size - 1)];
    int count = 0;
    /* Create all receives that will be posted first */
    for (int i = 0; i < size; ++i) {
      if (i != rank) {
        requests[count] = Request::irecv_init(static_cast<char *>(recvbuf) + recvdisps[i] * recvext,
                                          recvcounts[i], recvtype, i, system_tag, comm);
        count++;
      }else{
        XBT_DEBUG("<%d> skip request creation [src = %d, recvcounts[src] = %d]", rank, i, recvcounts[i]);
      }
    }
    /* Now create all sends  */
    for (int i = 0; i < size; ++i) {
      if (i != rank) {
      requests[count] = Request::isend_init(static_cast<const char *>(sendbuf) + senddisps[i] * sendext,
                                        sendcounts[i], sendtype, i, system_tag, comm);
      count++;
      }else{
        XBT_DEBUG("<%d> skip request creation [dst = %d, sendcounts[dst] = %d]", rank, i, sendcounts[i]);
      }
    }
    /* Wait for them all. */
    Request::startall(count, requests);
    (*request)->set_nbc_requests(requests, count);
  }
  return err;
}

int colls::ialltoallw(const void* sendbuf, const int* sendcounts, const int* senddisps, const MPI_Datatype* sendtypes,
                      void* recvbuf, const int* recvcounts, const int* recvdisps, const MPI_Datatype* recvtypes,
                      MPI_Comm comm, MPI_Request* request, int external)
{
  const int system_tag = COLL_TAG_ALLTOALLW-external;

  /* Initialize. */
  int rank = comm->rank();
  int size = comm->size();
  (*request) = new Request( nullptr, 0, MPI_BYTE,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT);
  /* Local copy from self */
  int err = (sendcounts[rank]>0 && recvcounts[rank]) ? Datatype::copy(static_cast<const char *>(sendbuf) + senddisps[rank], sendcounts[rank], sendtypes[rank],
                               static_cast<char *>(recvbuf) + recvdisps[rank], recvcounts[rank], recvtypes[rank]): MPI_SUCCESS;
  if (err == MPI_SUCCESS && size > 1) {
    /* Initiate all send/recv to/from others. */
    auto* requests = new MPI_Request[2 * (size - 1)];
    int count = 0;
    /* Create all receives that will be posted first */
    for (int i = 0; i < size; ++i) {
      if (i != rank) {
        requests[count] = Request::irecv_init(static_cast<char *>(recvbuf) + recvdisps[i],
                                          recvcounts[i], recvtypes[i], i, system_tag, comm);
        count++;
      }else{
        XBT_DEBUG("<%d> skip request creation [src = %d, recvcounts[src] = %d]", rank, i, recvcounts[i]);
      }
    }
    /* Now create all sends  */
    for (int i = 0; i < size; ++i) {
      if (i != rank) {
      requests[count] = Request::isend_init(static_cast<const char *>(sendbuf) + senddisps[i] ,
                                        sendcounts[i], sendtypes[i], i, system_tag, comm);
      count++;
      }else{
        XBT_DEBUG("<%d> skip request creation [dst = %d, sendcounts[dst] = %d]", rank, i, sendcounts[i]);
      }
    }
    /* Wait for them all. */
    Request::startall(count, requests);
    (*request)->set_nbc_requests(requests, count);
  }
  return err;
}

int colls::igather(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, int recvcount,
                   MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request, int external)
{
  const int system_tag = COLL_TAG_GATHER-external;
  MPI_Aint lb = 0;
  MPI_Aint recvext = 0;

  int rank = comm->rank();
  int size = comm->size();
  (*request) = new Request( nullptr, 0, MPI_BYTE,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT);
  if(rank != root) {
    // Send buffer to root
    auto* requests = new MPI_Request[1];
    requests[0]=Request::isend(sendbuf, sendcount, sendtype, root, system_tag, comm);
    (*request)->set_nbc_requests(requests, 1);
  } else {
    recvtype->extent(&lb, &recvext);
    // Local copy from root
    Datatype::copy(sendbuf, sendcount, sendtype, static_cast<char*>(recvbuf) + root * recvcount * recvext,
                       recvcount, recvtype);
    // Receive buffers from senders
    auto* requests = new MPI_Request[size - 1];
    int index = 0;
    for (int src = 0; src < size; src++) {
      if(src != root) {
        requests[index] = Request::irecv_init(static_cast<char*>(recvbuf) + src * recvcount * recvext, recvcount, recvtype,
                                          src, system_tag, comm);
        index++;
      }
    }
    // Wait for completion of irecv's.
    Request::startall(size - 1, requests);
    (*request)->set_nbc_requests(requests, size - 1);
  }
  return MPI_SUCCESS;
}

int colls::igatherv(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, const int* recvcounts,
                    const int* displs, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request,
                    int external)
{
  int system_tag = COLL_TAG_GATHERV-external;
  MPI_Aint lb = 0;
  MPI_Aint recvext = 0;
  
  int rank = comm->rank();
  int size = comm->size();
  (*request) = new Request( nullptr, 0, MPI_BYTE,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT);
  if (rank != root) {
    // Send buffer to root
    auto* requests = new MPI_Request[1];
    requests[0]=Request::isend(sendbuf, sendcount, sendtype, root, system_tag, comm);
    (*request)->set_nbc_requests(requests, 1);
  } else {
    recvtype->extent(&lb, &recvext);
    // Local copy from root
    Datatype::copy(sendbuf, sendcount, sendtype, static_cast<char*>(recvbuf) + displs[root] * recvext,
                       recvcounts[root], recvtype);
    // Receive buffers from senders
    auto* requests = new MPI_Request[size - 1];
    int index = 0;
    for (int src = 0; src < size; src++) {
      if(src != root) {
        requests[index] = Request::irecv_init(static_cast<char*>(recvbuf) + displs[src] * recvext,
                          recvcounts[src], recvtype, src, system_tag, comm);
        index++;
      }
    }
    // Wait for completion of irecv's.
    Request::startall(size - 1, requests);
    (*request)->set_nbc_requests(requests, size - 1);
  }
  return MPI_SUCCESS;
}
int colls::iscatterv(const void* sendbuf, const int* sendcounts, const int* displs, MPI_Datatype sendtype,
                     void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request,
                     int external)
{
  int system_tag = COLL_TAG_SCATTERV-external;
  MPI_Aint lb = 0;
  MPI_Aint sendext = 0;

  int rank = comm->rank();
  int size = comm->size();
  (*request) = new Request( nullptr, 0, MPI_BYTE,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT);
  if(rank != root) {
    // Recv buffer from root
    auto* requests = new MPI_Request[1];
    requests[0]=Request::irecv(recvbuf, recvcount, recvtype, root, system_tag, comm);
    (*request)->set_nbc_requests(requests, 1);
  } else {
    sendtype->extent(&lb, &sendext);
    // Local copy from root
    if(recvbuf!=MPI_IN_PLACE){
      Datatype::copy(static_cast<const char *>(sendbuf) + displs[root] * sendext, sendcounts[root],
                       sendtype, recvbuf, recvcount, recvtype);
    }
    // Send buffers to receivers
    auto* requests = new MPI_Request[size - 1];
    int index = 0;
    for (int dst = 0; dst < size; dst++) {
      if (dst != root) {
        requests[index] = Request::isend_init(static_cast<const char *>(sendbuf) + displs[dst] * sendext, sendcounts[dst],
                            sendtype, dst, system_tag, comm);
        index++;
      }
    }
    // Wait for completion of isend's.
    Request::startall(size - 1, requests);
    (*request)->set_nbc_requests(requests, size - 1);
  }
  return MPI_SUCCESS;
}

int colls::ireduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root,
                   MPI_Comm comm, MPI_Request* request, int external)
{
  const int system_tag = COLL_TAG_REDUCE-external;
  MPI_Aint lb = 0;
  MPI_Aint dataext = 0;

  const void* real_sendbuf = sendbuf;

  int rank = comm->rank();
  int size = comm->size();

  if (size <= 0)
    return MPI_ERR_COMM;

  unsigned char* tmp_sendbuf = nullptr;
  if( sendbuf == MPI_IN_PLACE ) {
    tmp_sendbuf = smpi_get_tmp_sendbuffer(count * datatype->get_extent());
    Datatype::copy(recvbuf, count, datatype, tmp_sendbuf, count, datatype);
    real_sendbuf = tmp_sendbuf;
  }

  if(rank == root){
    (*request) =  new Request( recvbuf, count, datatype,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT, op);
  }
  else
    (*request) = new Request( nullptr, count, datatype,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT);

  if(rank != root) {
    // Send buffer to root
    auto* requests        = new MPI_Request[1];
    requests[0]           = Request::isend(real_sendbuf, count, datatype, root, system_tag, comm);
    (*request)->set_nbc_requests(requests, 1);
  } else {
    datatype->extent(&lb, &dataext);
    // Local copy from root
    if (real_sendbuf != nullptr && recvbuf != nullptr)
      Datatype::copy(real_sendbuf, count, datatype, recvbuf, count, datatype);
    // Receive buffers from senders
    auto* requests = new MPI_Request[size - 1];
    int index = 0;
    for (int src = 0; src < size; src++) {
      if (src != root) {
        requests[index] =
          Request::irecv_init(smpi_get_tmp_sendbuffer(count * dataext), count, datatype, src, system_tag, comm);
        index++;
      }
    }
    // Wait for completion of irecv's.
    Request::startall(size - 1, requests);
    (*request)->set_nbc_requests(requests, size - 1);
  }	
  if( sendbuf == MPI_IN_PLACE ) {
    smpi_free_tmp_buffer(tmp_sendbuf);
  }
  return MPI_SUCCESS;
}

int colls::iallreduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm,
                      MPI_Request* request, int external)
{

  const int system_tag = COLL_TAG_ALLREDUCE-external;
  MPI_Aint lb = 0;
  MPI_Aint dataext = 0;

  int rank = comm->rank();
  int size = comm->size();
  (*request) = new Request( recvbuf, count, datatype,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT, op);
  // FIXME: check for errors
  datatype->extent(&lb, &dataext);
  // Local copy from self
  Datatype::copy(sendbuf, count, datatype, recvbuf, count, datatype);
  // Send/Recv buffers to/from others;
  auto* requests = new MPI_Request[2 * (size - 1)];
  int index = 0;
  for (int other = 0; other < size; other++) {
    if(other != rank) {
      requests[index] = Request::isend_init(sendbuf, count, datatype, other, system_tag,comm);
      index++;
      requests[index] = Request::irecv_init(smpi_get_tmp_sendbuffer(count * dataext), count, datatype,
                                        other, system_tag, comm);
      index++;
    }
  }
  Request::startall(2 * (size - 1), requests);
  (*request)->set_nbc_requests(requests, 2 * (size - 1));
  return MPI_SUCCESS;
}

int colls::iscan(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm,
                 MPI_Request* request, int external)
{
  int system_tag = -888-external;
  MPI_Aint lb      = 0;
  MPI_Aint dataext = 0;

  int rank = comm->rank();
  int size = comm->size();
  (*request) = new Request( recvbuf, count, datatype,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT, op);
  datatype->extent(&lb, &dataext);

  // Local copy from self
  Datatype::copy(sendbuf, count, datatype, recvbuf, count, datatype);

  // Send/Recv buffers to/from others
  auto* requests = new MPI_Request[size - 1];
  int index = 0;
  for (int other = 0; other < rank; other++) {
    requests[index] = Request::irecv_init(smpi_get_tmp_sendbuffer(count * dataext), count, datatype, other, system_tag, comm);
    index++;
  }
  for (int other = rank + 1; other < size; other++) {
    requests[index] = Request::isend_init(sendbuf, count, datatype, other, system_tag, comm);
    index++;
  }
  // Wait for completion of all comms.
  Request::startall(size - 1, requests);
  (*request)->set_nbc_requests(requests, size - 1);
  return MPI_SUCCESS;
}

int colls::iexscan(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm,
                   MPI_Request* request, int external)
{
  int system_tag = -888-external;
  MPI_Aint lb         = 0;
  MPI_Aint dataext    = 0;
  int rank = comm->rank();
  int size = comm->size();
  (*request) = new Request( recvbuf, count, datatype,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT, op);
  datatype->extent(&lb, &dataext);
  if(rank != 0)
    memset(recvbuf, 0, count*dataext);

  // Send/Recv buffers to/from others
  auto* requests = new MPI_Request[size - 1];
  int index = 0;
  for (int other = 0; other < rank; other++) {
    requests[index] = Request::irecv_init(smpi_get_tmp_sendbuffer(count * dataext), count, datatype, other, system_tag, comm);
    index++;
  }
  for (int other = rank + 1; other < size; other++) {
    requests[index] = Request::isend_init(sendbuf, count, datatype, other, system_tag, comm);
    index++;
  }
  // Wait for completion of all comms.
  Request::startall(size - 1, requests);
  (*request)->set_nbc_requests(requests, size - 1);
  return MPI_SUCCESS;
}

int colls::ireduce_scatter(const void* sendbuf, void* recvbuf, const int* recvcounts, MPI_Datatype datatype, MPI_Op op,
                           MPI_Comm comm, MPI_Request* request, int external)
{
  // Version where each process performs the reduce for its own part. Alltoall pattern for comms.
  const int system_tag = COLL_TAG_REDUCE_SCATTER-external;
  MPI_Aint lb = 0;
  MPI_Aint dataext = 0;

  int rank = comm->rank();
  int size = comm->size();
  int count=recvcounts[rank];
  (*request) = new Request( recvbuf, count, datatype,
                         rank,rank, system_tag, comm, MPI_REQ_PERSISTENT, op);
  datatype->extent(&lb, &dataext);

  // Send/Recv buffers to/from others;
  auto* requests = new MPI_Request[2 * (size - 1)];
  int index = 0;
  int recvdisp=0;
  for (int other = 0; other < size; other++) {
    if(other != rank) {
      requests[index] = Request::isend_init(static_cast<const char *>(sendbuf) + recvdisp * dataext, recvcounts[other], datatype, other, system_tag,comm);
      XBT_VERB("sending with recvdisp %d", recvdisp);
      index++;
      requests[index] = Request::irecv_init(smpi_get_tmp_sendbuffer(count * dataext), count, datatype,
                                        other, system_tag, comm);
      index++;
    }else{
      Datatype::copy(static_cast<const char *>(sendbuf) + recvdisp * dataext, count, datatype, recvbuf, count, datatype);
    }
    recvdisp+=recvcounts[other];
  }
  Request::startall(2 * (size - 1), requests);
  (*request)->set_nbc_requests(requests, 2 * (size - 1));
  return MPI_SUCCESS;
}

}
}
