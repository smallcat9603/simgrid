/* Copyright (c) 2007-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "private.hpp"
#include "smpi_coll.hpp"
#include "smpi_comm.hpp"
#include "smpi_request.hpp"
#include "smpi_datatype_derived.hpp"
#include "smpi_op.hpp"
#include "src/smpi/include/smpi_actor.hpp"

#include <vector>

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(smpi_pmpi);

static const void* smpi_get_in_place_buf(const void* inplacebuf, const void* otherbuf,
                                         std::vector<unsigned char>& tmp_sendbuf, int count, MPI_Datatype datatype)
{
  if (inplacebuf == MPI_IN_PLACE) {
    tmp_sendbuf.resize(count * datatype->get_extent());
    simgrid::smpi::Datatype::copy(otherbuf, count, datatype, tmp_sendbuf.data(), count, datatype);
    return tmp_sendbuf.data();
  }else{
    return inplacebuf;
  }
}
/* PMPI User level calls */

int PMPI_Barrier(MPI_Comm comm)
{
  return PMPI_Ibarrier(comm, MPI_REQUEST_IGNORED);
}

int PMPI_Ibarrier(MPI_Comm comm, MPI_Request *request)
{
  CHECK_COMM(1)
  CHECK_REQUEST(2)

  smpi_bench_end();
  int rank = simgrid::s4u::this_actor::get_pid();
  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Barrier" : "PMPI_Ibarrier",
                     new simgrid::instr::NoOpTIData(request == MPI_REQUEST_IGNORED ? "barrier" : "ibarrier"));
  if (request == MPI_REQUEST_IGNORED) {
    simgrid::smpi::colls::barrier(comm);
    // Barrier can be used to synchronize RMA calls. Finish all requests from comm before.
    comm->finish_rma_calls();
  } else
    simgrid::smpi::colls::ibarrier(comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
  return PMPI_Ibcast(buf, count, datatype, root, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Ibcast(void *buf, int count, MPI_Datatype datatype, 
                   int root, MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(5)
  CHECK_BUFFER(1, buf, count)
  CHECK_COUNT(2, count)
  CHECK_TYPE(3, datatype)
  CHECK_ROOT(4)
  CHECK_REQUEST(6)

  smpi_bench_end();
  int rank = simgrid::s4u::this_actor::get_pid();
  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Bcast" : "PMPI_Ibcast",
                     new simgrid::instr::CollTIData(request == MPI_REQUEST_IGNORED ? "bcast" : "ibcast", root, -1.0,
                                                    datatype->is_replayable() ? count : count * datatype->size(), -1,
                                                    simgrid::smpi::Datatype::encode(datatype), ""));
  if (comm->size() > 1) {
    if (request == MPI_REQUEST_IGNORED)
      simgrid::smpi::colls::bcast(buf, count, datatype, root, comm);
    else
      simgrid::smpi::colls::ibcast(buf, count, datatype, root, comm, request);
  } else {
    if (request != MPI_REQUEST_IGNORED)
      *request = MPI_REQUEST_NULL;
  }

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,void *recvbuf, int recvcount, MPI_Datatype recvtype,
                int root, MPI_Comm comm){
  return PMPI_Igather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Igather(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, int recvcount,
                 MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(8)
  if(sendbuf != MPI_IN_PLACE){
    CHECK_BUFFER(1,sendbuf, sendcount)
    CHECK_COUNT(2, sendcount)
    CHECK_TYPE(3, sendtype)
  }
  if(comm->rank() == root){
    CHECK_TYPE(6, recvtype)
    CHECK_COUNT(5, recvcount)
    CHECK_BUFFER(4, recvbuf, recvcount)
  }
  CHECK_ROOT(7)
  CHECK_REQUEST(9)

  smpi_bench_end();
  const void* real_sendbuf   = sendbuf;
  int real_sendcount         = sendcount;
  MPI_Datatype real_sendtype = sendtype;
  if ((comm->rank() == root) && (sendbuf == MPI_IN_PLACE)) {
    real_sendcount = 0;
    real_sendtype  = recvtype;
  }
  int rank = simgrid::s4u::this_actor::get_pid();

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Gather" : "PMPI_Igather",
                     new simgrid::instr::CollTIData(
                         request == MPI_REQUEST_IGNORED ? "gather" : "igather", root, -1.0,
                         real_sendtype->is_replayable() ? real_sendcount : real_sendcount * real_sendtype->size(),
                         (comm->rank() != root || recvtype->is_replayable()) ? recvcount : recvcount * recvtype->size(),
                         simgrid::smpi::Datatype::encode(real_sendtype), simgrid::smpi::Datatype::encode(recvtype)));
  if (request == MPI_REQUEST_IGNORED)
    simgrid::smpi::colls::gather(real_sendbuf, real_sendcount, real_sendtype, recvbuf, recvcount, recvtype, root, comm);
  else
    simgrid::smpi::colls::igather(real_sendbuf, real_sendcount, real_sendtype, recvbuf, recvcount, recvtype, root, comm,
                                  request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, const int *recvcounts, const int *displs,
                MPI_Datatype recvtype, int root, MPI_Comm comm){
  return PMPI_Igatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Igatherv(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, const int* recvcounts, const int* displs,
                  MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(9)
  CHECK_BUFFER(1, sendbuf, sendcount)
  if(sendbuf != MPI_IN_PLACE){
    CHECK_TYPE(3, sendtype)
    CHECK_COUNT(2, sendcount)
  }
  if(comm->rank() == root){
    CHECK_TYPE(6, recvtype)
    CHECK_NULL(5, MPI_ERR_COUNT, recvcounts)
    CHECK_NULL(6, MPI_ERR_ARG, displs)
  }
  CHECK_ROOT(8)
  CHECK_REQUEST(10)

  if (comm->rank() == root){
    for (int i = 0; i < comm->size(); i++) {
      CHECK_COUNT(5, recvcounts[i])
      CHECK_BUFFER(4,recvbuf,recvcounts[i])
    }
  }

  smpi_bench_end();
  const void* real_sendbuf   = sendbuf;
  int real_sendcount         = sendcount;
  MPI_Datatype real_sendtype = sendtype;
  if ((comm->rank() == root) && (sendbuf == MPI_IN_PLACE)) {
    real_sendcount = 0;
    real_sendtype  = recvtype;
  }

  int rank         = simgrid::s4u::this_actor::get_pid();
  int dt_size_recv = recvtype->is_replayable() ? 1 : recvtype->size();

  auto* trace_recvcounts = new std::vector<int>();
  if (comm->rank() == root) {
    for (int i = 0; i < comm->size(); i++) // copy data to avoid bad free
      trace_recvcounts->push_back(recvcounts[i] * dt_size_recv);
  }

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Gatherv" : "PMPI_Igatherv",
                     new simgrid::instr::VarCollTIData(
                         request == MPI_REQUEST_IGNORED ? "gatherv" : "igatherv", root,
                         real_sendtype->is_replayable() ? real_sendcount : real_sendcount * real_sendtype->size(),
                         nullptr, dt_size_recv, trace_recvcounts, simgrid::smpi::Datatype::encode(real_sendtype),
                         simgrid::smpi::Datatype::encode(recvtype)));
  if (request == MPI_REQUEST_IGNORED)
    simgrid::smpi::colls::gatherv(real_sendbuf, real_sendcount, real_sendtype, recvbuf, recvcounts, displs, recvtype,
                                  root, comm);
  else
    simgrid::smpi::colls::igatherv(real_sendbuf, real_sendcount, real_sendtype, recvbuf, recvcounts, displs, recvtype,
                                   root, comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                   void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm){
  return PMPI_Iallgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Iallgather(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, int recvcount,
                    MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(7)
  CHECK_BUFFER(1, sendbuf, sendcount)
  CHECK_BUFFER(4, recvbuf, recvcount)
  if(sendbuf != MPI_IN_PLACE){
    CHECK_COUNT(2, sendcount)
    CHECK_TYPE(3, sendtype)
  }
  CHECK_TYPE(6, recvtype)
  CHECK_COUNT(5, recvcount)
  CHECK_REQUEST(8)

  smpi_bench_end();
  if (sendbuf == MPI_IN_PLACE) {
    sendbuf   = static_cast<char*>(recvbuf) + recvtype->get_extent() * recvcount * comm->rank();
    sendcount = recvcount;
    sendtype  = recvtype;
  }
  int rank = simgrid::s4u::this_actor::get_pid();

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Allgather" : "PMPI_Iallggather",
                     new simgrid::instr::CollTIData(
                         request == MPI_REQUEST_IGNORED ? "allgather" : "iallgather", -1, -1.0,
                         sendtype->is_replayable() ? sendcount : sendcount * sendtype->size(),
                         recvtype->is_replayable() ? recvcount : recvcount * recvtype->size(),
                         simgrid::smpi::Datatype::encode(sendtype), simgrid::smpi::Datatype::encode(recvtype)));
  if (request == MPI_REQUEST_IGNORED)
    simgrid::smpi::colls::allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
  else
    simgrid::smpi::colls::iallgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                   void *recvbuf, const int *recvcounts, const int *displs, MPI_Datatype recvtype, MPI_Comm comm){
  return PMPI_Iallgatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Iallgatherv(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, const int* recvcounts, const int* displs,
                     MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(8)
  CHECK_BUFFER(1, sendbuf, sendcount)
  if(sendbuf != MPI_IN_PLACE)
    CHECK_TYPE(3, sendtype)
  CHECK_TYPE(6, recvtype)
  CHECK_NULL(5, MPI_ERR_COUNT, recvcounts)
  CHECK_NULL(6, MPI_ERR_ARG, displs)
  if(sendbuf != MPI_IN_PLACE)
    CHECK_COUNT(2, sendcount)
  CHECK_REQUEST(9)
  for (int i = 0; i < comm->size(); i++) {
    CHECK_COUNT(5, recvcounts[i])
    CHECK_BUFFER(4, recvbuf, recvcounts[i])
  }

  smpi_bench_end();
  if (sendbuf == MPI_IN_PLACE) {
    sendbuf   = static_cast<char*>(recvbuf) + recvtype->get_extent() * displs[comm->rank()];
    sendcount = recvcounts[comm->rank()];
    sendtype  = recvtype;
  }
  int rank         = simgrid::s4u::this_actor::get_pid();
  int dt_size_recv = recvtype->is_replayable() ? 1 : recvtype->size();

  auto* trace_recvcounts = new std::vector<int>();
  for (int i = 0; i < comm->size(); i++) { // copy data to avoid bad free
    trace_recvcounts->push_back(recvcounts[i] * dt_size_recv);
  }

  TRACE_smpi_comm_in(
      rank, request == MPI_REQUEST_IGNORED ? "PMPI_Allgatherv" : "PMPI_Iallgatherv",
      new simgrid::instr::VarCollTIData(request == MPI_REQUEST_IGNORED ? "allgatherv" : "iallgatherv", -1,
                                        sendtype->is_replayable() ? sendcount : sendcount * sendtype->size(), nullptr,
                                        dt_size_recv, trace_recvcounts, simgrid::smpi::Datatype::encode(sendtype),
                                        simgrid::smpi::Datatype::encode(recvtype)));
  if (request == MPI_REQUEST_IGNORED)
    simgrid::smpi::colls::allgatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm);
  else
    simgrid::smpi::colls::iallgatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm,
                                      request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm){
  return PMPI_Iscatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Iscatter(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, int recvcount,
                  MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(8)
  if(comm->rank() == root){
    CHECK_BUFFER(1, sendbuf, sendcount)
    CHECK_COUNT(2, sendcount)
    CHECK_TYPE(3, sendtype)
  }
  if(recvbuf != MPI_IN_PLACE){
    CHECK_BUFFER(4, recvbuf, recvcount)
    CHECK_COUNT(5, recvcount)
    CHECK_TYPE(6, recvtype)
  }
  CHECK_ROOT(8)
  CHECK_REQUEST(9)

  smpi_bench_end();
  if (recvbuf == MPI_IN_PLACE) {
    recvtype  = sendtype;
    recvcount = sendcount;
  }
  int rank = simgrid::s4u::this_actor::get_pid();

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Scatter" : "PMPI_Iscatter",
                     new simgrid::instr::CollTIData(
                         request == MPI_REQUEST_IGNORED ? "scatter" : "iscatter", root, -1.0,
                         (comm->rank() != root || sendtype->is_replayable()) ? sendcount : sendcount * sendtype->size(),
                         recvtype->is_replayable() ? recvcount : recvcount * recvtype->size(),
                         simgrid::smpi::Datatype::encode(sendtype), simgrid::smpi::Datatype::encode(recvtype)));
  if (request == MPI_REQUEST_IGNORED)
    simgrid::smpi::colls::scatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
  else
    simgrid::smpi::colls::iscatter(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Scatterv(const void *sendbuf, const int *sendcounts, const int *displs,
                 MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm){
  return PMPI_Iscatterv(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Iscatterv(const void* sendbuf, const int* sendcounts, const int* displs, MPI_Datatype sendtype, void* recvbuf, int recvcount,
                   MPI_Datatype recvtype, int root, MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(9)
  if(recvbuf != MPI_IN_PLACE){
    CHECK_BUFFER(4, recvbuf, recvcount)
    CHECK_COUNT(5, recvcount)
    CHECK_TYPE(7, recvtype)
  }
  CHECK_ROOT(9)
  CHECK_REQUEST(10)
  if (comm->rank() == root) {
    CHECK_NULL(2, MPI_ERR_COUNT, sendcounts)
    CHECK_NULL(3, MPI_ERR_ARG, displs)
    CHECK_TYPE(4, sendtype)
    for (int i = 0; i < comm->size(); i++){
      CHECK_BUFFER(1, sendbuf, sendcounts[i])
      CHECK_COUNT(2, sendcounts[i])
    }
    if (recvbuf == MPI_IN_PLACE) {
      recvtype  = sendtype;
      recvcount = sendcounts[comm->rank()];
    }
  }

  smpi_bench_end();

  int rank         = simgrid::s4u::this_actor::get_pid();
  int dt_size_send = sendtype->is_replayable() ? 1 : sendtype->size();

  auto* trace_sendcounts = new std::vector<int>();
  if (comm->rank() == root) {
    for (int i = 0; i < comm->size(); i++) { // copy data to avoid bad free
      trace_sendcounts->push_back(sendcounts[i] * dt_size_send);
    }
  }

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Scatterv" : "PMPI_Iscatterv",
                     new simgrid::instr::VarCollTIData(
                         request == MPI_REQUEST_IGNORED ? "scatterv" : "iscatterv", root, dt_size_send,
                         trace_sendcounts, recvtype->is_replayable() ? recvcount : recvcount * recvtype->size(),
                         nullptr, simgrid::smpi::Datatype::encode(sendtype),
                         simgrid::smpi::Datatype::encode(recvtype)));
  if (request == MPI_REQUEST_IGNORED)
    simgrid::smpi::colls::scatterv(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm);
  else
    simgrid::smpi::colls::iscatterv(sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm,
                                    request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
  return PMPI_Ireduce(sendbuf, recvbuf, count, datatype, op, root, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Ireduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(7)
  CHECK_BUFFER(1, sendbuf, count)
  if(comm->rank() == root)
    CHECK_BUFFER(5, recvbuf, count)
  CHECK_TYPE(4, datatype)
  CHECK_COUNT(3, count)
  CHECK_OP(5)
  CHECK_ROOT(7)
  CHECK_REQUEST(8)

  smpi_bench_end();
  int rank = simgrid::s4u::this_actor::get_pid();

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Reduce" : "PMPI_Ireduce",
                     new simgrid::instr::CollTIData(request == MPI_REQUEST_IGNORED ? "reduce" : "ireduce", root, 0,
                                                    datatype->is_replayable() ? count : count * datatype->size(), -1,
                                                    simgrid::smpi::Datatype::encode(datatype), ""));
  if (request == MPI_REQUEST_IGNORED)
    simgrid::smpi::colls::reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
  else
    simgrid::smpi::colls::ireduce(sendbuf, recvbuf, count, datatype, op, root, comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Reduce_local(const void* inbuf, void* inoutbuf, int count, MPI_Datatype datatype, MPI_Op op)
{
  CHECK_BUFFER(1, inbuf, count)
  CHECK_BUFFER(2, inoutbuf, count)
  CHECK_TYPE(4, datatype)
  CHECK_COUNT(3, count)
  CHECK_OP(5)

  smpi_bench_end();
  op->apply(inbuf, inoutbuf, &count, datatype);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
  return PMPI_Iallreduce(sendbuf, recvbuf, count, datatype, op, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Iallreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *request)
{
  CHECK_COMM(6)
  CHECK_BUFFER(1, sendbuf, count)
  CHECK_BUFFER(2, recvbuf, count)
  CHECK_TYPE(4, datatype)
  CHECK_COUNT(3, count)
  CHECK_REQUEST(7)
  CHECK_OP(5)

  smpi_bench_end();
  std::vector<unsigned char> tmp_sendbuf;
  const void* real_sendbuf = smpi_get_in_place_buf(sendbuf, recvbuf, tmp_sendbuf, count, datatype);

  int rank = simgrid::s4u::this_actor::get_pid();

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Allreduce" : "PMPI_Iallreduce",
                     new simgrid::instr::CollTIData(request == MPI_REQUEST_IGNORED ? "allreduce" : "iallreduce", -1, 0,
                                                    datatype->is_replayable() ? count : count * datatype->size(), -1,
                                                    simgrid::smpi::Datatype::encode(datatype), ""));

  if (request == MPI_REQUEST_IGNORED)
    simgrid::smpi::colls::allreduce(real_sendbuf, recvbuf, count, datatype, op, comm);
  else
    simgrid::smpi::colls::iallreduce(real_sendbuf, recvbuf, count, datatype, op, comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Scan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
  return PMPI_Iscan(sendbuf, recvbuf, count, datatype, op, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Iscan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(6)
  CHECK_BUFFER(1,sendbuf,count)
  CHECK_BUFFER(2,recvbuf,count)
  CHECK_TYPE(4, datatype)
  CHECK_COUNT(3, count)
  CHECK_REQUEST(7)
  CHECK_OP(5)

  smpi_bench_end();
  int rank         = simgrid::s4u::this_actor::get_pid();
  std::vector<unsigned char> tmp_sendbuf;
  const void* real_sendbuf = smpi_get_in_place_buf(sendbuf, recvbuf, tmp_sendbuf, count, datatype);

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Scan" : "PMPI_Iscan",
                     new simgrid::instr::Pt2PtTIData(request == MPI_REQUEST_IGNORED ? "scan" : "iscan", -1,
                                                     datatype->is_replayable() ? count : count * datatype->size(),
                                                     simgrid::smpi::Datatype::encode(datatype)));

  int retval;
  if (request == MPI_REQUEST_IGNORED)
    retval = simgrid::smpi::colls::scan(real_sendbuf, recvbuf, count, datatype, op, comm);
  else
    retval = simgrid::smpi::colls::iscan(real_sendbuf, recvbuf, count, datatype, op, comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return retval;
}

int PMPI_Exscan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
  return PMPI_Iexscan(sendbuf, recvbuf, count, datatype, op, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Iexscan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request* request){
  CHECK_COMM(6)
  CHECK_BUFFER(1, sendbuf, count)
  CHECK_BUFFER(2, recvbuf, count)
  CHECK_TYPE(4, datatype)
  CHECK_COUNT(3, count)
  CHECK_REQUEST(7)
  CHECK_OP(5)

  smpi_bench_end();
  int rank         = simgrid::s4u::this_actor::get_pid();
  std::vector<unsigned char> tmp_sendbuf;
  const void* real_sendbuf = smpi_get_in_place_buf(sendbuf, recvbuf, tmp_sendbuf, count, datatype);

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Exscan" : "PMPI_Iexscan",
                     new simgrid::instr::Pt2PtTIData(request == MPI_REQUEST_IGNORED ? "exscan" : "iexscan", -1,
                                                     datatype->is_replayable() ? count : count * datatype->size(),
                                                     simgrid::smpi::Datatype::encode(datatype)));

  int retval;
  if (request == MPI_REQUEST_IGNORED)
    retval = simgrid::smpi::colls::exscan(real_sendbuf, recvbuf, count, datatype, op, comm);
  else
    retval = simgrid::smpi::colls::iexscan(real_sendbuf, recvbuf, count, datatype, op, comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return retval;
}

int PMPI_Reduce_scatter(const void *sendbuf, void *recvbuf, const int *recvcounts, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
  return PMPI_Ireduce_scatter(sendbuf, recvbuf, recvcounts, datatype, op, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Ireduce_scatter(const void *sendbuf, void *recvbuf, const int *recvcounts, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm, MPI_Request *request)
{
  CHECK_COMM(6)
  CHECK_TYPE(4, datatype)
  CHECK_NULL(3, MPI_ERR_COUNT, recvcounts)
  CHECK_REQUEST(7)
  CHECK_OP(5)
  for (int i = 0; i < comm->size(); i++) {
    CHECK_COUNT(3, recvcounts[i])
    CHECK_BUFFER(1, sendbuf, recvcounts[i])
    CHECK_BUFFER(2, recvbuf, recvcounts[i])
  }

  smpi_bench_end();
  int rank                           = simgrid::s4u::this_actor::get_pid();
  auto* trace_recvcounts             = new std::vector<int>();
  int dt_send_size                   = datatype->is_replayable() ? 1 : datatype->size();
  int totalcount                     = 0;

  for (int i = 0; i < comm->size(); i++) { // copy data to avoid bad free
    trace_recvcounts->push_back(recvcounts[i] * dt_send_size);
    totalcount += recvcounts[i];
  }
  std::vector<unsigned char> tmp_sendbuf;
  const void* real_sendbuf = smpi_get_in_place_buf(sendbuf, recvbuf, tmp_sendbuf, totalcount, datatype);

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Reduce_scatter" : "PMPI_Ireduce_scatter",
                     new simgrid::instr::VarCollTIData(
                         request == MPI_REQUEST_IGNORED ? "reducescatter" : "ireducescatter", -1, dt_send_size, nullptr,
                         -1, trace_recvcounts, simgrid::smpi::Datatype::encode(datatype), ""));

  if (request == MPI_REQUEST_IGNORED)
    simgrid::smpi::colls::reduce_scatter(real_sendbuf, recvbuf, recvcounts, datatype, op, comm);
  else
    simgrid::smpi::colls::ireduce_scatter(real_sendbuf, recvbuf, recvcounts, datatype, op, comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Reduce_scatter_block(const void *sendbuf, void *recvbuf, int recvcount,
                              MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
  return PMPI_Ireduce_scatter_block(sendbuf, recvbuf, recvcount, datatype, op, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Ireduce_scatter_block(const void* sendbuf, void* recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op,
                               MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(6)
  CHECK_BUFFER(1, sendbuf, recvcount)
  CHECK_BUFFER(2, recvbuf, recvcount)
  CHECK_TYPE(4, datatype)
  CHECK_COUNT(3, recvcount)
  CHECK_REQUEST(7)
  CHECK_OP(5)

  smpi_bench_end();
  int count = comm->size();

  int rank                           = simgrid::s4u::this_actor::get_pid();
  int dt_send_size                   = datatype->is_replayable() ? 1 : datatype->size();
  auto* trace_recvcounts             = new std::vector<int>(recvcount * dt_send_size); // copy data to avoid bad free
  std::vector<unsigned char> tmp_sendbuf;
  const void* real_sendbuf = smpi_get_in_place_buf(sendbuf, recvbuf, tmp_sendbuf, recvcount * count, datatype);

  TRACE_smpi_comm_in(
      rank, request == MPI_REQUEST_IGNORED ? "PMPI_Reduce_scatter_block" : "PMPI_Ireduce_scatter_block",
      new simgrid::instr::VarCollTIData(request == MPI_REQUEST_IGNORED ? "reducescatter" : "ireducescatter", -1, 0,
                                        nullptr, -1, trace_recvcounts, simgrid::smpi::Datatype::encode(datatype), ""));

  std::vector<int> recvcounts(count);
  for (int i      = 0; i < count; i++)
    recvcounts[i] = recvcount;
  if (request == MPI_REQUEST_IGNORED)
    simgrid::smpi::colls::reduce_scatter(real_sendbuf, recvbuf, recvcounts.data(), datatype, op, comm);
  else
    simgrid::smpi::colls::ireduce_scatter(real_sendbuf, recvbuf, recvcounts.data(), datatype, op, comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return MPI_SUCCESS;
}

int PMPI_Alltoall(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, int recvcount,
                  MPI_Datatype recvtype, MPI_Comm comm){
  return PMPI_Ialltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Ialltoall(const void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, int recvcount,
                   MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(7)
  CHECK_BUFFER(1, sendbuf, sendcount)
  CHECK_BUFFER(4, recvbuf, recvcount)
  if(sendbuf != MPI_IN_PLACE)
    CHECK_TYPE(3, sendtype)
  CHECK_TYPE(6, recvtype)
  CHECK_COUNT(5, recvcount)
  if(sendbuf != MPI_IN_PLACE)
    CHECK_COUNT(2, sendcount)
  CHECK_COUNT(5, recvcount)
  CHECK_REQUEST(8)

  smpi_bench_end();
  int rank                 = simgrid::s4u::this_actor::get_pid();
  int real_sendcount         = sendcount;
  MPI_Datatype real_sendtype = sendtype;

  std::vector<unsigned char> tmp_sendbuf;
  const void* real_sendbuf = smpi_get_in_place_buf(sendbuf, recvbuf, tmp_sendbuf, recvcount * comm->size(), recvtype);

  if (sendbuf == MPI_IN_PLACE) {
    real_sendcount = recvcount;
    real_sendtype  = recvtype;
  }

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Alltoall" : "PMPI_Ialltoall",
                     new simgrid::instr::CollTIData(
                         request == MPI_REQUEST_IGNORED ? "alltoall" : "ialltoall", -1, -1.0,
                         real_sendtype->is_replayable() ? real_sendcount : real_sendcount * real_sendtype->size(),
                         recvtype->is_replayable() ? recvcount : recvcount * recvtype->size(),
                         simgrid::smpi::Datatype::encode(real_sendtype), simgrid::smpi::Datatype::encode(recvtype)));
  int retval;
  if (request == MPI_REQUEST_IGNORED)
    retval =
        simgrid::smpi::colls::alltoall(real_sendbuf, real_sendcount, real_sendtype, recvbuf, recvcount, recvtype, comm);
  else
    retval = simgrid::smpi::colls::ialltoall(real_sendbuf, real_sendcount, real_sendtype, recvbuf, recvcount, recvtype,
                                             comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return retval;
}

int PMPI_Alltoallv(const void* sendbuf, const int* sendcounts, const int* senddispls, MPI_Datatype sendtype, void* recvbuf,
                   const int* recvcounts, const int* recvdispls, MPI_Datatype recvtype, MPI_Comm comm)
{
  return PMPI_Ialltoallv(sendbuf, sendcounts, senddispls, sendtype, recvbuf, recvcounts, recvdispls, recvtype, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Ialltoallv(const void* sendbuf, const int* sendcounts, const int* senddispls, MPI_Datatype sendtype, void* recvbuf,
                    const int* recvcounts, const int* recvdispls, MPI_Datatype recvtype, MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(9)
  if(sendbuf != MPI_IN_PLACE){
    CHECK_NULL(2, MPI_ERR_COUNT, sendcounts)
    CHECK_NULL(3, MPI_ERR_ARG, senddispls)
    CHECK_TYPE(4, sendtype)
  }
  CHECK_TYPE(8, recvtype)
  CHECK_NULL(6, MPI_ERR_COUNT, recvcounts)
  CHECK_NULL(7, MPI_ERR_ARG, recvdispls)
  CHECK_REQUEST(10)

  int rank = simgrid::s4u::this_actor::get_pid();
  int size = comm->size();
  for (int i = 0; i < size; i++) {
    if(sendbuf != MPI_IN_PLACE){
      CHECK_BUFFER(1, sendbuf, sendcounts[i])
      CHECK_COUNT(2, sendcounts[i])
    }
    CHECK_BUFFER(5, recvbuf, recvcounts[i])
    CHECK_COUNT(6, recvcounts[i])
  }

  smpi_bench_end();
  int send_size                      = 0;
  int recv_size                      = 0;
  auto* trace_sendcounts             = new std::vector<int>();
  auto* trace_recvcounts             = new std::vector<int>();
  int dt_size_recv                   = recvtype->size();

  const int* real_sendcounts = sendcounts;
  const int* real_senddispls  = senddispls;
  MPI_Datatype real_sendtype = sendtype;
  int maxsize              = 0;
  for (int i = 0; i < size; i++) { // copy data to avoid bad free
    recv_size += recvcounts[i] * dt_size_recv;
    trace_recvcounts->push_back(recvcounts[i] * dt_size_recv);
    if (((recvdispls[i] + recvcounts[i]) * dt_size_recv) > maxsize)
      maxsize = (recvdispls[i] + recvcounts[i]) * dt_size_recv;
  }

  std::vector<unsigned char> tmp_sendbuf;
  std::vector<int> tmp_sendcounts;
  std::vector<int> tmp_senddispls;
  const void* real_sendbuf = smpi_get_in_place_buf(sendbuf, recvbuf, tmp_sendbuf, maxsize, MPI_CHAR);
  if (sendbuf == MPI_IN_PLACE) {
    tmp_sendcounts.assign(recvcounts, recvcounts + size);
    real_sendcounts = tmp_sendcounts.data();
    tmp_senddispls.assign(recvdispls, recvdispls + size);
    real_senddispls = tmp_senddispls.data();
    real_sendtype  = recvtype;
  }

  int dt_size_send = real_sendtype->size();

  for (int i = 0; i < size; i++) { // copy data to avoid bad free
    send_size += real_sendcounts[i] * dt_size_send;
    trace_sendcounts->push_back(real_sendcounts[i] * dt_size_send);
  }

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Alltoallv" : "PMPI_Ialltoallv",
                     new simgrid::instr::VarCollTIData(request == MPI_REQUEST_IGNORED ? "alltoallv" : "ialltoallv", -1,
                                                       send_size, trace_sendcounts, recv_size, trace_recvcounts,
                                                       simgrid::smpi::Datatype::encode(real_sendtype),
                                                       simgrid::smpi::Datatype::encode(recvtype)));

  int retval;
  if (request == MPI_REQUEST_IGNORED)
    retval = simgrid::smpi::colls::alltoallv(real_sendbuf, real_sendcounts, real_senddispls, real_sendtype, recvbuf,
                                             recvcounts, recvdispls, recvtype, comm);
  else
    retval = simgrid::smpi::colls::ialltoallv(real_sendbuf, real_sendcounts, real_senddispls, real_sendtype, recvbuf,
                                              recvcounts, recvdispls, recvtype, comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return retval;
}

int PMPI_Alltoallw(const void* sendbuf, const int* sendcounts, const int* senddispls, const MPI_Datatype* sendtypes, void* recvbuf,
                   const int* recvcounts, const int* recvdispls, const MPI_Datatype* recvtypes, MPI_Comm comm)
{
  return PMPI_Ialltoallw(sendbuf, sendcounts, senddispls, sendtypes, recvbuf, recvcounts, recvdispls, recvtypes, comm, MPI_REQUEST_IGNORED);
}

int PMPI_Ialltoallw(const void* sendbuf, const int* sendcounts, const int* senddispls, const MPI_Datatype* sendtypes, void* recvbuf,
                    const int* recvcounts, const int* recvdispls, const MPI_Datatype* recvtypes, MPI_Comm comm, MPI_Request* request)
{
  CHECK_COMM(9)
  if(sendbuf != MPI_IN_PLACE){
    CHECK_NULL(2, MPI_ERR_COUNT, sendcounts)
    CHECK_NULL(3, MPI_ERR_ARG, senddispls)
    CHECK_NULL(4, MPI_ERR_TYPE, sendtypes)
  }
  CHECK_NULL(6, MPI_ERR_COUNT, recvcounts)
  CHECK_NULL(7, MPI_ERR_ARG, recvdispls)
  CHECK_NULL(8, MPI_ERR_TYPE, recvtypes)
  CHECK_REQUEST(10)
  int rank = simgrid::s4u::this_actor::get_pid();
  int size = comm->size();
  for (int i = 0; i < size; i++) {
    if(sendbuf != MPI_IN_PLACE){
      CHECK_BUFFER(1, sendbuf, sendcounts[i])
      CHECK_COUNT(2, sendcounts[i])
      CHECK_TYPE(4, sendtypes[i])
    }
    CHECK_BUFFER(5, recvbuf, recvcounts[i])
    CHECK_COUNT(6, recvcounts[i])
    CHECK_TYPE(8, recvtypes[i])
  }

  smpi_bench_end();

  int send_size                      = 0;
  int recv_size                      = 0;
  auto* trace_sendcounts             = new std::vector<int>();
  auto* trace_recvcounts             = new std::vector<int>();

  const int* real_sendcounts         = sendcounts;
  const int* real_senddispls          = senddispls;
  const MPI_Datatype* real_sendtypes = sendtypes;
  unsigned long maxsize      = 0;
  for (int i = 0; i < size; i++) { // copy data to avoid bad free
    if (recvtypes[i] == MPI_DATATYPE_NULL) {
      delete trace_recvcounts;
      delete trace_sendcounts;
      return MPI_ERR_TYPE;
    }
    recv_size += recvcounts[i] * recvtypes[i]->size();
    trace_recvcounts->push_back(recvcounts[i] * recvtypes[i]->size());
    if ((recvdispls[i] + (recvcounts[i] * recvtypes[i]->size())) > maxsize)
      maxsize = recvdispls[i] + (recvcounts[i] * recvtypes[i]->size());
  }

  std::vector<unsigned char> tmp_sendbuf;
  std::vector<int> tmp_sendcounts;
  std::vector<int> tmp_senddispls;
  std::vector<MPI_Datatype> tmp_sendtypes;
  const void* real_sendbuf = smpi_get_in_place_buf(sendbuf, recvbuf, tmp_sendbuf, maxsize, MPI_CHAR);
  if (sendbuf == MPI_IN_PLACE) {
    tmp_sendcounts.assign(recvcounts, recvcounts + size);
    real_sendcounts = tmp_sendcounts.data();
    tmp_senddispls.assign(recvdispls, recvdispls + size);
    real_senddispls = tmp_senddispls.data();
    tmp_sendtypes.assign(recvtypes, recvtypes + size);
    real_sendtypes = tmp_sendtypes.data();
  }

  for (int i = 0; i < size; i++) { // copy data to avoid bad free
    send_size += real_sendcounts[i] * real_sendtypes[i]->size();
    trace_sendcounts->push_back(real_sendcounts[i] * real_sendtypes[i]->size());
  }

  TRACE_smpi_comm_in(rank, request == MPI_REQUEST_IGNORED ? "PMPI_Alltoallw" : "PMPI_Ialltoallw",
                     new simgrid::instr::VarCollTIData(request == MPI_REQUEST_IGNORED ? "alltoallv" : "ialltoallv", -1,
                                                       send_size, trace_sendcounts, recv_size, trace_recvcounts,
                                                       simgrid::smpi::Datatype::encode(real_sendtypes[0]),
                                                       simgrid::smpi::Datatype::encode(recvtypes[0])));

  int retval;
  if (request == MPI_REQUEST_IGNORED)
    retval = simgrid::smpi::colls::alltoallw(real_sendbuf, real_sendcounts, real_senddispls, real_sendtypes, recvbuf,
                                             recvcounts, recvdispls, recvtypes, comm);
  else
    retval = simgrid::smpi::colls::ialltoallw(real_sendbuf, real_sendcounts, real_senddispls, real_sendtypes, recvbuf,
                                              recvcounts, recvdispls, recvtypes, comm, request);

  TRACE_smpi_comm_out(rank);
  smpi_bench_begin();
  return retval;
}
