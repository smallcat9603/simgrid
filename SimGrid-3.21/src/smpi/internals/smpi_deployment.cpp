/* Copyright (c) 2004-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "smpi_host.hpp"
#include "private.hpp"
#include "simgrid/s4u/Engine.hpp"
#include "smpi_comm.hpp"
#include <map>

namespace simgrid {
namespace smpi {
namespace app {

class Instance {
public:
  Instance(const std::string name, int max_no_processes, int process_count, MPI_Comm comm,
           simgrid::s4u::Barrier* finalization_barrier)
      : name(name)
      , size(max_no_processes)
      , present_processes(0)
      , comm_world(comm)
      , finalization_barrier(finalization_barrier)
  { }

  const std::string name;
  int size;
  int present_processes;
  MPI_Comm comm_world;
  simgrid::s4u::Barrier* finalization_barrier;
};
}
}
}

using simgrid::smpi::app::Instance;

static std::map<std::string, Instance> smpi_instances;
extern int process_count; // How many processes have been allocated over all instances?

/** @ingroup smpi_simulation
 * @brief Registers a running instance of a MPI program.
 *
 * @param name the reference name of the function.
 * @param code the main mpi function (must have a int ..(int argc, char *argv[])) prototype
 * @param num_processes the size of the instance we want to deploy
 */
void SMPI_app_instance_register(const char *name, xbt_main_func_t code, int num_processes)
{
  if (code != nullptr) { // When started with smpirun, we will not execute a function
    simgrid::s4u::Engine::get_instance()->register_function(name, code);
  }

  static int already_called = 0;
  if (not already_called) {
    already_called = 1;
    std::vector<simgrid::s4u::Host*> list = simgrid::s4u::Engine::get_instance()->get_all_hosts();
    for (auto const& host : list) {
      host->extension_set(new simgrid::smpi::Host(host));
    }
  }

  Instance instance(std::string(name), num_processes, process_count, MPI_COMM_NULL,
                    new simgrid::s4u::Barrier(num_processes));
  MPI_Group group     = new simgrid::smpi::Group(instance.size);
  instance.comm_world = new simgrid::smpi::Comm(group, nullptr);
//  FIXME : using MPI_Attr_put with MPI_UNIVERSE_SIZE is forbidden and we make it a no-op (which triggers a warning as MPI_ERR_ARG is returned). 
//  Directly calling Comm::attr_put breaks for now, as MPI_UNIVERSE_SIZE,is <0
//  instance.comm_world->attr_put<simgrid::smpi::Comm>(MPI_UNIVERSE_SIZE, reinterpret_cast<void*>(instance.size));

  process_count+=num_processes;

  smpi_instances.insert(std::pair<std::string, Instance>(name, instance));
}

void smpi_deployment_register_process(const std::string instance_id, int rank, simgrid::s4u::ActorPtr actor)
{
  Instance& instance = smpi_instances.at(instance_id);

  instance.present_processes++;
  instance.comm_world->group()->set_mapping(actor, rank);
}

MPI_Comm* smpi_deployment_comm_world(const std::string instance_id)
{
  if (smpi_instances.empty()) { // no instance registered, we probably used smpirun.
    return nullptr;
  }
  Instance& instance = smpi_instances.at(instance_id);
  return &instance.comm_world;
}

simgrid::s4u::Barrier* smpi_deployment_finalization_barrier(const std::string instance_id)
{
  if (smpi_instances.empty()) { // no instance registered, we probably used smpirun.
    return nullptr;
  }
  Instance& instance = smpi_instances.at(instance_id);
  return instance.finalization_barrier;
}

void smpi_deployment_cleanup_instances(){
  for (auto const& item : smpi_instances) {
    Instance instance = item.second;
    delete instance.finalization_barrier;
    simgrid::smpi::Comm::destroy(instance.comm_world);
  }
  smpi_instances.clear();
}
