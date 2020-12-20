/* Copyright (c) 2013-2020. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "../colls_private.hpp"

namespace simgrid{
namespace smpi{

// Allgather-Non-Topology-Specific-Logical-Ring algorithm
int
allgather__NTSLR(const void *sbuf, int scount, MPI_Datatype stype,
                 void *rbuf, int rcount, MPI_Datatype rtype,
                 MPI_Comm comm)
{
  MPI_Aint rextent, sextent;
  MPI_Status status;
  int i, to, from, rank, size;
  int send_offset, recv_offset;
  int tag = COLL_TAG_ALLGATHER;

  rank = comm->rank();
  size = comm->size();
  rextent = rtype->get_extent();
  sextent = stype->get_extent();

  // irregular case use default MPI functions
  if (scount * sextent != rcount * rextent) {
    XBT_WARN("MPI_allgather_NTSLR use default MPI_allgather.");
    allgather__default(sbuf, scount, stype, rbuf, rcount, rtype, comm);
    return MPI_SUCCESS;
  }

  // topo non-specific
  to = (rank + 1) % size;
  from = (rank + size - 1) % size;

  //copy a single segment from sbuf to rbuf
  send_offset = rank * scount * sextent;

  Request::sendrecv(sbuf, scount, stype, rank, tag,
               (char *)rbuf + send_offset, rcount, rtype, rank, tag,
               comm, &status);


  //start sending logical ring message
  int increment = scount * sextent;
  for (i = 0; i < size - 1; i++) {
    send_offset = ((rank - i + size) % size) * increment;
    recv_offset = ((rank - i - 1 + size) % size) * increment;
    Request::sendrecv((char *) rbuf + send_offset, scount, stype, to, tag + i,
                 (char *) rbuf + recv_offset, rcount, rtype, from, tag + i,
                 comm, &status);
  }

  return MPI_SUCCESS;
}


}
}
