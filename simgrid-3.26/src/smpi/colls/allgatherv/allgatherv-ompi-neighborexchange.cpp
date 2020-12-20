/* Copyright (c) 2013-2020. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/*
 * ompi_coll_tuned_allgatherv_intra_neighborexchange
 *
 * Function:     allgatherv using N/2 steps (O(N))
 * Accepts:      Same arguments as MPI_Allgatherv
 * Returns:      MPI_SUCCESS or error code
 *
 * Description:  Neighbor Exchange algorithm for allgather adapted for
 *               allgatherv.
 *               Described by Chen et.al. in
 *               "Performance Evaluation of Allgather Algorithms on
 *                Terascale Linux Cluster with Fast Ethernet",
 *               Proceedings of the Eighth International Conference on
 *               High-Performance Computing inn Asia-Pacific Region
 *               (HPCASIA'05), 2005
 *
 *               Rank r exchanges message with one of its neighbors and
 *               forwards the data further in the next step.
 *
 *               No additional memory requirements.
 *
 * Limitations:  Algorithm works only on even number of processes.
 *               For odd number of processes we switch to ring algorithm.
 *
 * Example on 6 nodes:
 *  Initial state
 *    #     0      1      2      3      4      5
 *         [0]    [ ]    [ ]    [ ]    [ ]    [ ]
 *         [ ]    [1]    [ ]    [ ]    [ ]    [ ]
 *         [ ]    [ ]    [2]    [ ]    [ ]    [ ]
 *         [ ]    [ ]    [ ]    [3]    [ ]    [ ]
 *         [ ]    [ ]    [ ]    [ ]    [4]    [ ]
 *         [ ]    [ ]    [ ]    [ ]    [ ]    [5]
 *   Step 0:
 *    #     0      1      2      3      4      5
 *         [0]    [0]    [ ]    [ ]    [ ]    [ ]
 *         [1]    [1]    [ ]    [ ]    [ ]    [ ]
 *         [ ]    [ ]    [2]    [2]    [ ]    [ ]
 *         [ ]    [ ]    [3]    [3]    [ ]    [ ]
 *         [ ]    [ ]    [ ]    [ ]    [4]    [4]
 *         [ ]    [ ]    [ ]    [ ]    [5]    [5]
 *   Step 1:
 *    #     0      1      2      3      4      5
 *         [0]    [0]    [0]    [ ]    [ ]    [0]
 *         [1]    [1]    [1]    [ ]    [ ]    [1]
 *         [ ]    [2]    [2]    [2]    [2]    [ ]
 *         [ ]    [3]    [3]    [3]    [3]    [ ]
 *         [4]    [ ]    [ ]    [4]    [4]    [4]
 *         [5]    [ ]    [ ]    [5]    [5]    [5]
 *   Step 2:
 *    #     0      1      2      3      4      5
 *         [0]    [0]    [0]    [0]    [0]    [0]
 *         [1]    [1]    [1]    [1]    [1]    [1]
 *         [2]    [2]    [2]    [2]    [2]    [2]
 *         [3]    [3]    [3]    [3]    [3]    [3]
 *         [4]    [4]    [4]    [4]    [4]    [4]
 *         [5]    [5]    [5]    [5]    [5]    [5]
 */

#include "../colls_private.hpp"

namespace simgrid{
namespace smpi{

int
allgatherv__ompi_neighborexchange(const void *sbuf, int scount,
                                  MPI_Datatype sdtype,
                                  void* rbuf, const int *rcounts, const int *rdispls,
                                  MPI_Datatype rdtype,
                                  MPI_Comm comm)
{
    int line = -1;
    int rank, size;
    int neighbor[2], offset_at_step[2], recv_data_from[2], send_data_from;

    int i, even_rank;
    int err = 0;
    ptrdiff_t slb, rlb, sext, rext;
    char *tmpsend = nullptr, *tmprecv = nullptr;

    size = comm->size();
    rank = comm->rank();

    if (size % 2) {
        XBT_DEBUG("allgatherv__ompi_neighborexchange WARNING: odd size %d, switching to ring algorithm",
                     size);
        return allgatherv__ring(sbuf, scount, sdtype,
                                                     rbuf, rcounts,
                                                     rdispls, rdtype,
                                                     comm);
    }

    XBT_DEBUG(
                 "coll:tuned:allgatherv_ompi_neighborexchange rank %d", rank);

    err = sdtype->extent(&slb, &sext);
    if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

    err = rdtype->extent(&rlb, &rext);
    if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }

    /* Initialization step:
       - if send buffer is not MPI_IN_PLACE, copy send buffer to
       the appropriate block of receive buffer
    */
    tmprecv = (char*) rbuf + rdispls[rank] * rext;
    if (MPI_IN_PLACE != sbuf) {
        tmpsend = (char*) sbuf;
        err = Datatype::copy(tmpsend, scount, sdtype,
                              tmprecv, rcounts[rank], rdtype);
        if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl;  }
    }

    /* Determine neighbors, order in which blocks will arrive, etc. */
    even_rank = !(rank % 2);
    if (even_rank) {
        neighbor[0] = (rank + 1) % size;
        neighbor[1] = (rank - 1 + size) % size;
        recv_data_from[0] = rank;
        recv_data_from[1] = rank;
        offset_at_step[0] = (+2);
        offset_at_step[1] = (-2);
    } else {
        neighbor[0] = (rank - 1 + size) % size;
        neighbor[1] = (rank + 1) % size;
        recv_data_from[0] = neighbor[0];
        recv_data_from[1] = neighbor[0];
        offset_at_step[0] = (-2);
        offset_at_step[1] = (+2);
    }

    /* Communication loop:
       - First step is special: exchange a single block with neighbor[0].
       - Rest of the steps:
       update recv_data_from according to offset, and
       exchange two blocks with appropriate neighbor.
       the send location becomes previous receive location.
       Note, we need to create indexed datatype to send and receive these
       blocks properly.
    */
    tmprecv = (char*)rbuf + rdispls[neighbor[0]] * rext;
    tmpsend = (char*)rbuf + rdispls[rank] * rext;
    Request::sendrecv(tmpsend, rcounts[rank], rdtype,
                                   neighbor[0], COLL_TAG_ALLGATHERV,
                                   tmprecv, rcounts[neighbor[0]], rdtype,
                                   neighbor[0], COLL_TAG_ALLGATHERV,
                                   comm, MPI_STATUS_IGNORE);





    /* Determine initial sending counts and displacements*/
    if (even_rank) {
        send_data_from = rank;
    } else {
        send_data_from = recv_data_from[0];
    }

    for (i = 1; i < (size / 2); i++) {
        MPI_Datatype new_rdtype, new_sdtype;
        int new_scounts[2], new_sdispls[2], new_rcounts[2], new_rdispls[2];
        const int i_parity = i % 2;
        recv_data_from[i_parity] =
            (recv_data_from[i_parity] + offset_at_step[i_parity] + size) % size;

        /* Create new indexed types for sending and receiving.
           We are sending data from ranks (send_data_from) and (send_data_from+1)
           We are receiving data from ranks (recv_data_from[i_parity]) and
           (recv_data_from[i_parity]+1).
        */

        new_scounts[0] = rcounts[send_data_from];
        new_scounts[1] = rcounts[(send_data_from + 1)];
        new_sdispls[0] = rdispls[send_data_from];
        new_sdispls[1] = rdispls[(send_data_from + 1)];
        err = Datatype::create_indexed(2, new_scounts, new_sdispls, rdtype,
                                      &new_sdtype);
        if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }
        new_sdtype->commit();

        new_rcounts[0] = rcounts[recv_data_from[i_parity]];
        new_rcounts[1] = rcounts[(recv_data_from[i_parity] + 1)];
        new_rdispls[0] = rdispls[recv_data_from[i_parity]];
        new_rdispls[1] = rdispls[(recv_data_from[i_parity] + 1)];
        err = Datatype::create_indexed(2, new_rcounts, new_rdispls, rdtype,
                                      &new_rdtype);
        if (MPI_SUCCESS != err) { line = __LINE__; goto err_hndl; }
        new_rdtype->commit();

        tmprecv = (char*)rbuf;
        tmpsend = (char*)rbuf;

        /* Sendreceive */
        Request::sendrecv(tmpsend, 1, new_sdtype, neighbor[i_parity],
                                       COLL_TAG_ALLGATHERV,
                                       tmprecv, 1, new_rdtype, neighbor[i_parity],
                                       COLL_TAG_ALLGATHERV,
                                       comm, MPI_STATUS_IGNORE);

        send_data_from = recv_data_from[i_parity];

        Datatype::unref(new_sdtype);
        Datatype::unref(new_rdtype);
    }

    return MPI_SUCCESS;

 err_hndl:
    XBT_DEBUG(  "%s:%4d\tError occurred %d, rank %2d",
                 __FILE__, line, err, rank);
    return err;
}

}
}
