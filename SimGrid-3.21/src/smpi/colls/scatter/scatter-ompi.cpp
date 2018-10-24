/* Copyright (c) 2013-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 *
 * Additional copyrights may follow
 */

#include "../coll_tuned_topo.hpp"
#include "../colls_private.hpp"

namespace simgrid{
namespace smpi{

int Coll_scatter_ompi_binomial::scatter(void* sbuf, int scount, MPI_Datatype sdtype, void* rbuf, int rcount,
                                        MPI_Datatype rdtype, int root, MPI_Comm comm)
{
    int line = -1;
    int i;
    int rank;
    int vrank;
    int size;
    int total_send = 0;
    char *ptmp     = NULL;
    char *tempbuf  = NULL;
    int err;
    ompi_coll_tree_t* bmtree;
    MPI_Status status;
    MPI_Aint sextent, slb, strue_lb, strue_extent;
    MPI_Aint rextent, rlb, rtrue_lb, rtrue_extent;

    size = comm->size();
    rank = comm->rank();

    XBT_DEBUG(
                 "Coll_scatter_ompi_binomial::scatter rank %d", rank);

    /* create the binomial tree */

//    COLL_TUNED_UPDATE_IN_ORDER_BMTREE( comm, tuned_module, root );
    bmtree =  ompi_coll_tuned_topo_build_in_order_bmtree( comm, root);//ompi_ data->cached_in_order_bmtree;

    sdtype->extent(&slb, &sextent);
    sdtype->extent(&strue_lb, &strue_extent);
    rdtype->extent(&rlb, &rextent);
    rdtype->extent(&rtrue_lb, &rtrue_extent);

    vrank = (rank - root + size) % size;

    if (rank == root) {
      if (0 == root) {
        /* root on 0, just use the send buffer */
        ptmp = (char*)sbuf;
        if (rbuf != MPI_IN_PLACE) {
          /* local copy to rbuf */
          err = Datatype::copy(sbuf, scount, sdtype, rbuf, rcount, rdtype);
          if (MPI_SUCCESS != err) {
            line = __LINE__;
            goto err_hndl;
          }
        }
      } else {
        /* root is not on 0, allocate temp buffer for send */
        tempbuf = (char*)smpi_get_tmp_sendbuffer(strue_extent + (scount * size - 1) * sextent);
        if (NULL == tempbuf) {
          err  = MPI_ERR_OTHER;
          line = __LINE__;
          goto err_hndl;
        }

        ptmp = tempbuf - slb;

        /* and rotate data so they will eventually in the right place */
        err = Datatype::copy((char*)sbuf + sextent * root * scount, scount * (size - root), sdtype, ptmp,
                             scount * (size - root), sdtype);
        if (MPI_SUCCESS != err) {
          line = __LINE__;
          goto err_hndl;
        }

        err = Datatype::copy((char*)sbuf, scount * root, sdtype, ptmp + sextent * scount * (size - root), scount * root,
                             sdtype);
        if (MPI_SUCCESS != err) {
          line = __LINE__;
          goto err_hndl;
        }

        if (rbuf != MPI_IN_PLACE) {
          /* local copy to rbuf */
          err = Datatype::copy(ptmp, scount, sdtype, rbuf, rcount, rdtype);
          if (MPI_SUCCESS != err) {
            line = __LINE__;
            goto err_hndl;
          }
        }
      }
      total_send = scount;
    } else if (not(vrank % 2)) {
      /* non-root, non-leaf nodes, allocate temp buffer for recv
       * the most we need is rcount*size/2 */
      tempbuf = (char*)smpi_get_tmp_recvbuffer(rtrue_extent + (rcount * size - 1) * rextent);
      if (NULL == tempbuf) {
        err  = MPI_ERR_OTHER;
        line = __LINE__;
        goto err_hndl;
      }

      ptmp = tempbuf - rlb;

      sdtype     = rdtype;
      scount     = rcount;
      sextent    = rextent;
      total_send = scount;
    } else {
      /* leaf nodes, just use rbuf */
      ptmp = (char*)rbuf;
    }

    if (not(vrank % 2)) {
      if (rank != root) {
        /* recv from parent on non-root */
        Request::recv(ptmp, rcount * size, rdtype, bmtree->tree_prev, COLL_TAG_SCATTER, comm, &status);
        /* local copy to rbuf */
        Datatype::copy(ptmp, scount, sdtype, rbuf, rcount, rdtype);
      }
      /* send to children on all non-leaf */
      for (i = 0; i < bmtree->tree_nextsize; i++) {
        int mycount = 0, vkid;
        /* figure out how much data I have to send to this child */
        vkid    = (bmtree->tree_next[i] - root + size) % size;
        mycount = vkid - vrank;
        if (mycount > (size - vkid))
          mycount = size - vkid;
        mycount *= scount;

        Request::send(ptmp + total_send * sextent, mycount, sdtype, bmtree->tree_next[i], COLL_TAG_SCATTER, comm);

        total_send += mycount;
      }

    } else {
      /* recv from parent on leaf nodes */
      Request::recv(ptmp, rcount, rdtype, bmtree->tree_prev, COLL_TAG_SCATTER, comm, &status);
    }

    if (NULL != tempbuf)
      smpi_free_tmp_buffer(tempbuf);
    // not FIXME : store the tree, as done in ompi, instead of calculating it each time ?
    ompi_coll_tuned_topo_destroy_tree(&bmtree);

    return MPI_SUCCESS;

 err_hndl:
    if (NULL != tempbuf)
      free(tempbuf);

    XBT_DEBUG("%s:%4d\tError occurred %d, rank %2d", __FILE__, line, err, rank);
    return err;
}

/*
 * Linear functions are copied from the BASIC coll module
 * they do not segment the message and are simple implementations
 * but for some small number of nodes and/or small data sizes they
 * are just as fast as tuned/tree based segmenting operations
 * and as such may be selected by the decision functions
 * These are copied into this module due to the way we select modules
 * in V1. i.e. in V2 we will handle this differently and so will not
 * have to duplicate code.
 * JPG following the examples from other coll_tuned implementations. Dec06.
 */

/* copied function (with appropriate renaming) starts here */
/*
 *  scatter_intra
 *
 *  Function:  - basic scatter operation
 *  Accepts:  - same arguments as MPI_Scatter()
 *  Returns:  - MPI_SUCCESS or error code
 */
int Coll_scatter_ompi_basic_linear::scatter(void* sbuf, int scount, MPI_Datatype sdtype, void* rbuf, int rcount,
                                            MPI_Datatype rdtype, int root, MPI_Comm comm)
{
    int i, rank, size, err;
    char *ptmp;
    ptrdiff_t lb, incr;

    /* Initialize */

    rank = comm->rank();
    size = comm->size();

    /* If not root, receive data. */

    if (rank != root) {
        Request::recv(rbuf, rcount, rdtype, root,
                                COLL_TAG_SCATTER,
                                comm, MPI_STATUS_IGNORE);
        return MPI_SUCCESS;
    }

    /* I am the root, loop sending data. */

    err = sdtype->extent(&lb, &incr);
    if (MPI_SUCCESS != err) {
        return MPI_ERR_OTHER;
    }

    incr *= scount;
    for (i = 0, ptmp = (char *) sbuf; i < size; ++i, ptmp += incr) {

        /* simple optimization */

        if (i == rank) {
            if (MPI_IN_PLACE != rbuf) {
                err =
                    Datatype::copy(ptmp, scount, sdtype, rbuf, rcount,
                                    rdtype);
            }
        } else {
            Request::send(ptmp, scount, sdtype, i,
                                    COLL_TAG_SCATTER,
                                     comm);
        }
        if (MPI_SUCCESS != err) {
            return err;
        }
    }

    /* All done */

    return MPI_SUCCESS;
}

}
}
