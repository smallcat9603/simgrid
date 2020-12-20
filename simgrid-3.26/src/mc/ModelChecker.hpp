/* Copyright (c) 2007-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_MC_MODEL_CHECKER_HPP
#define SIMGRID_MC_MODEL_CHECKER_HPP

#include "src/mc/remote/CheckerSide.hpp"
#include "src/mc/sosp/PageStore.hpp"
#include "xbt/base.h"

#include <memory>
#include <set>
#include <string>

namespace simgrid {
namespace mc {

/** State of the model-checker (global variables for the model checker)
 */
class ModelChecker {
  CheckerSide checker_side_;
  /** String pool for host names */
  std::set<std::string> hostnames_;
  // This is the parent snapshot of the current state:
  PageStore page_store_{500};
  std::unique_ptr<RemoteSimulation> remote_simulation_;
  Checker* checker_ = nullptr;

public:
  ModelChecker(ModelChecker const&) = delete;
  ModelChecker& operator=(ModelChecker const&) = delete;
  explicit ModelChecker(std::unique_ptr<RemoteSimulation> remote_simulation, int sockfd);

  RemoteSimulation& get_remote_simulation() { return *remote_simulation_; }
  Channel& channel() { return checker_side_.get_channel(); }
  PageStore& page_store()
  {
    return page_store_;
  }

  std::string const& get_host_name(std::string const& hostname)
  {
    return *this->hostnames_.insert(hostname).first;
  }

  void start();
  void shutdown();
  void resume(simgrid::mc::RemoteSimulation& get_remote_simulation);
  void wait_for_requests();
  void handle_simcall(Transition const& transition);

  XBT_ATTRIB_NORETURN void exit(int status);

  bool checkDeadlock();

  Checker* getChecker() const { return checker_; }
  void setChecker(Checker* checker) { checker_ = checker; }

private:
  void setup_ignore();
  bool handle_message(const char* buffer, ssize_t size);
  void handle_waitpid();

public:
  unsigned long visited_states = 0;
  unsigned long executed_transitions = 0;
};

}
}

#endif
