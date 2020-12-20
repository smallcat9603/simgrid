/* Copyright (c) 2007-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/s4u/Host.hpp"
#include "smx_private.hpp"
#include "src/kernel/EngineImpl.hpp"
#include "src/surf/xml/platf_private.hpp" // FIXME: KILLME. There must be a better way than mimicking XML here
#include <simgrid/engine.h>
#include <simgrid/s4u/Engine.hpp>

#include <string>
#include <vector>

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(simix_deployment, simix, "Logging specific to SIMIX (deployment)");

void SIMIX_init_application() // XBT_ATTRIB_DEPRECATED_v329
{
  sg_platf_exit();
  sg_platf_init();
}

void SIMIX_launch_application(const std::string& file) // XBT_ATTRIB_DEPRECATED_v329
{
  simgrid_load_deployment(file.c_str());
}
void SIMIX_function_register(const std::string& name, xbt_main_func_t code) // XBT_ATTRIB_DEPRECATED_v329
{
  simgrid::s4u::Engine::get_instance()->register_function(name, code);
}
void SIMIX_function_register(const std::string& name,
                             void (*code)(std::vector<std::string>)) // XBT_ATTRIB_DEPRECATED_v329
{
  simgrid::s4u::Engine::get_instance()->register_function(name, code);
}
void SIMIX_function_register_default(xbt_main_func_t code) // XBT_ATTRIB_DEPRECATED_v329
{
  simgrid::s4u::Engine::get_instance()->register_default(code);
}

/** @brief Bypass the parser, get arguments, and set function to each process */
void SIMIX_process_set_function(const char* process_host, const char* process_function, xbt_dynar_t arguments,
                                double process_start_time, double process_kill_time) // XBT_ATTRIB_DEPRECATED_v329
{
  simgrid::kernel::routing::ActorCreationArgs actor;

  const simgrid::s4u::Host* host = sg_host_by_name(process_host);
  if (not host)
    throw std::invalid_argument(simgrid::xbt::string_printf("Host '%s' unknown", process_host));
  actor.host = process_host;
  actor.args.emplace_back(process_function);
  /* add arguments */
  unsigned int i;
  char *arg;
  xbt_dynar_foreach(arguments, i, arg) {
    actor.args.emplace_back(arg);
  }

  // Check we know how to handle this function name:
  const simgrid::kernel::actor::ActorCodeFactory& parse_code =
      simgrid::kernel::EngineImpl::get_instance()->get_function(process_function);
  xbt_assert(parse_code, "Function '%s' unknown", process_function);

  actor.function           = process_function;
  actor.kill_time          = process_kill_time;
  actor.start_time         = process_start_time;
  actor.restart_on_failure = false;
  sg_platf_new_actor(&actor);
}
