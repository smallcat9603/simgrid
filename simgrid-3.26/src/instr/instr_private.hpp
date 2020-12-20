/* Copyright (c) 2010-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef INSTR_PRIVATE_HPP
#define INSTR_PRIVATE_HPP

#include <xbt/base.h>

#include "simgrid/instr.h"
#include "simgrid/s4u/Actor.hpp"
#include "src/instr/instr_paje_containers.hpp"
#include "src/instr/instr_paje_events.hpp"
#include "src/instr/instr_paje_types.hpp"
#include "src/instr/instr_paje_values.hpp"

#include <fstream>
#include <iomanip> /** std::setprecision **/
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>

namespace simgrid {
namespace instr {
namespace paje {

void dump_generator_version();
void dump_comment_file(const std::string& filename);
void dump_header(bool basic, bool display_sizes);
} // namespace paje

/* Format of TRACING output.
 *   - paje is the regular format, that we all know
 *   - TI is a trick to reuse the tracing functions to generate a time independent trace during the execution. Such
 *     trace can easily be replayed with smpi_replay afterward. This trick should be removed and replaced by some code
 *     using the signal that we will create to cleanup the TRACING
 */
enum class TraceFormat { Paje, /*TimeIndependent*/ Ti };
extern TraceFormat trace_format;
extern int trace_precision;
extern double last_timestamp_to_dump;

void init();
void define_callbacks();

void platform_graph_export_graphviz(const std::string& output_filename);

void resource_set_utilization(const char* type, const char* name, const char* resource, const std::string& category,
                              double value, double now, double delta);
void dump_buffer(bool force);

class TIData {
  std::string name_;
  double amount_ = 0;

public:
  int endpoint                 = 0;
  int send_size                = 0;
  std::shared_ptr<std::vector<int>> sendcounts = nullptr;
  int recv_size                = 0;
  std::shared_ptr<std::vector<int>> recvcounts = nullptr;
  std::string send_type        = "";
  std::string recv_type        = "";

  // NoOpTI: init, finalize, test, wait, barrier
  explicit TIData(const std::string& name) : name_(name){};
  // CPuTI: compute, sleep (+ waitAny and waitall out of laziness)
  explicit TIData(const std::string& name, double amount) : name_(name), amount_(amount){};
  // Pt2PtTI: send, isend, ssend, issend, recv, irecv
  explicit TIData(const std::string& name, int endpoint, int size, const std::string& datatype)
      : name_(name), endpoint(endpoint), send_size(size), send_type(datatype){};
  // CollTI: bcast, reduce, allreduce, gather, scatter, allgather, alltoall
  explicit TIData(const std::string& name, int root, double amount, int send_size, int recv_size,
                  const std::string& send_type, const std::string& recv_type)
      : name_(name)
      , amount_(amount)
      , endpoint(root)
      , send_size(send_size)
      , recv_size(recv_size)
      , send_type(send_type)
      , recv_type(recv_type){};
  // VarCollTI: gatherv, scatterv, allgatherv, alltoallv (+ reducescatter out of laziness)
  explicit TIData(const std::string& name, int root, int send_size, std::vector<int>* sendcounts, int recv_size,
                  std::vector<int>* recvcounts, const std::string& send_type, const std::string& recv_type)
      : TIData(name, root, send_size, std::shared_ptr<std::vector<int>>(sendcounts), recv_size,
               std::shared_ptr<std::vector<int>>(recvcounts), send_type, recv_type){};

  explicit TIData(const std::string& name, int root, int send_size, std::shared_ptr<std::vector<int>> sendcounts,
                  int recv_size, std::shared_ptr<std::vector<int>> recvcounts, const std::string& send_type,
                  const std::string& recv_type)
      : name_(name)
      , endpoint(root)
      , send_size(send_size)
      , sendcounts(sendcounts)
      , recv_size(recv_size)
      , recvcounts(recvcounts)
      , send_type(send_type)
      , recv_type(recv_type){};

  virtual ~TIData() = default;

  const std::string& get_name() const { return name_; }
  double get_amount() const { return amount_; }
  virtual std::string print()        = 0;
  virtual std::string display_size() = 0;
};

class NoOpTIData : public TIData {
public:
  explicit NoOpTIData(const std::string& name) : TIData(name){};
  std::string print() override { return get_name(); }
  std::string display_size() override { return "NA"; }
};

class CpuTIData : public TIData {
public:
  explicit CpuTIData(const std::string& name, double amount) : TIData(name, amount){};
  std::string print() override
  {
    std::stringstream stream;
    stream << get_name() << " " << get_amount();
    return stream.str();
  }
  std::string display_size() override { return std::to_string(get_amount()); }
};

class Pt2PtTIData : public TIData {
  int tag = 0;

public:
  explicit Pt2PtTIData(const std::string& name, int endpoint, int size, int tag, const std::string& datatype)
      : TIData(name, endpoint, size, datatype), tag(tag){};

  explicit Pt2PtTIData(const std::string& name, int endpoint, int size, const std::string& datatype)
      : TIData(name, endpoint, size, datatype){};
  std::string print() override
  {
    std::stringstream stream;
    stream << get_name() << " " << endpoint << " ";
    stream << tag << " " << send_size << " " << send_type;
    return stream.str();
  }
  std::string display_size() override { return std::to_string(send_size); }
};

class CollTIData : public TIData {
public:
  explicit CollTIData(const std::string& name, int root, double amount, int send_size, int recv_size,
                      const std::string& send_type, const std::string& recv_type)
      : TIData(name, root, amount, send_size, recv_size, send_type, recv_type){};
  std::string print() override
  {
    std::stringstream stream;
    stream << get_name() << " " << send_size << " ";
    if (recv_size >= 0)
      stream << recv_size << " ";
    if (get_amount() >= 0.0)
      stream << get_amount() << " ";
    if (endpoint > 0 || (endpoint == 0 && not send_type.empty()))
      stream << endpoint << " ";
    stream << send_type << " " << recv_type;

    return stream.str();
  }
  std::string display_size() override { return std::to_string(send_size); }
};

class VarCollTIData : public TIData {
public:
  explicit VarCollTIData(const std::string& name, int root, int send_size, std::vector<int>* sendcounts, int recv_size,
                         std::vector<int>* recvcounts, const std::string& send_type, const std::string& recv_type)
      : TIData(name, root, send_size, sendcounts, recv_size, recvcounts, send_type, recv_type){};

  explicit VarCollTIData(const std::string& name, int root, int send_size, std::shared_ptr<std::vector<int>> sendcounts,
                         int recv_size, std::shared_ptr<std::vector<int>> recvcounts, const std::string& send_type,
                         const std::string& recv_type)
      : TIData(name, root, send_size, sendcounts, recv_size, recvcounts, send_type, recv_type){};

  std::string print() override
  {
    std::stringstream stream;
    stream << get_name() << " ";
    if (send_size >= 0)
      stream << send_size << " ";
    if (sendcounts != nullptr)
      for (auto count : *sendcounts)
        stream << count << " ";
    if (recv_size >= 0)
      stream << recv_size << " ";
    if (recvcounts != nullptr)
      for (auto count : *recvcounts)
        stream << count << " ";
    if (endpoint > 0 || (endpoint == 0 && not send_type.empty()))
      stream << endpoint << " ";
    stream << send_type << " " << recv_type;

    return stream.str();
  }
  std::string display_size() override { return std::to_string(send_size > 0 ? send_size : recv_size); }
};

/**
 * If we want to wait for a request of asynchronous communication, we need to be able
 * to identify this request. We do this by searching for a request identified by (src, dest, tag).
 */
class WaitTIData : public TIData {
  int src;
  int dest;
  int tag;

public:
  explicit WaitTIData(int src, int dest, int tag) : TIData("wait"), src(src), dest(dest), tag(tag){};

  std::string print() override
  {
    std::stringstream stream;
    stream << get_name() << " " << src << " " << dest << " " << tag;

    return stream.str();
  }

  std::string display_size() override { return "NA"; }
};

class AmpiMigrateTIData : public TIData {
  size_t memory_consumption;
public:
  explicit AmpiMigrateTIData(size_t memory_conso) : TIData("migrate"), memory_consumption(memory_conso) { };

  std::string print() override
  {
    std::stringstream stream;
    stream << get_name() << " " << memory_consumption;

    return stream.str();
  }

  std::string display_size() override { return "NA"; }
};
} // namespace instr
} // namespace simgrid

XBT_PRIVATE std::string instr_pid(simgrid::s4u::Actor const& proc);

extern XBT_PRIVATE std::set<std::string> created_categories;
extern XBT_PRIVATE std::set<std::string> declared_marks;
extern XBT_PRIVATE std::set<std::string> user_host_variables;
extern XBT_PRIVATE std::set<std::string> user_vm_variables;
extern XBT_PRIVATE std::set<std::string> user_link_variables;

/* from instr_config.c */
XBT_PRIVATE bool TRACE_needs_platform();
XBT_PRIVATE bool TRACE_is_enabled();
XBT_PRIVATE bool TRACE_platform();
XBT_PRIVATE bool TRACE_platform_topology();
XBT_PRIVATE bool TRACE_categorized();
XBT_PRIVATE bool TRACE_uncategorized();
XBT_PRIVATE bool TRACE_actor_is_enabled();
XBT_PRIVATE bool TRACE_vm_is_enabled();
XBT_PRIVATE bool TRACE_disable_link();
XBT_PRIVATE bool TRACE_disable_speed();
XBT_PRIVATE bool TRACE_display_sizes();

/* Public functions used in SMPI */
XBT_PUBLIC bool TRACE_smpi_is_enabled();
XBT_PUBLIC bool TRACE_smpi_is_grouped();
XBT_PUBLIC bool TRACE_smpi_is_computing();
XBT_PUBLIC bool TRACE_smpi_is_sleeping();
XBT_PUBLIC bool TRACE_smpi_view_internals();

/* instr_paje.c */
void instr_new_variable_type(const std::string& new_typename, const std::string& color);
void instr_new_user_variable_type(const std::string& father_type, const std::string& new_typename,
                                  const std::string& color);
void instr_new_user_state_type(const std::string& father_type, const std::string& new_typename);
void instr_new_value_for_user_state_type(const std::string& new_typename, const char* value, const std::string& color);

XBT_PRIVATE void TRACE_help();

#endif
