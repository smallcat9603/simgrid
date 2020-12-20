/* Copyright (c) 2014-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <simgrid/plugins/file_system.h>
#include <simgrid/s4u.hpp>
#include <string>

#define INMEGA (1024 * 1024)

XBT_LOG_NEW_DEFAULT_CATEGORY(remote_io, "Messages specific for this io example");

static void host(std::vector<std::string> args)
{
  simgrid::s4u::File file(args[1], nullptr);
  const char* filename = file.get_path();
  XBT_INFO("Opened file '%s'", filename);
  file.dump();
  XBT_INFO("Try to write %llu MiB to '%s'", file.size() / 1024, filename);
  sg_size_t write = file.write(file.size() * 1024);
  XBT_INFO("Have written %llu MiB to '%s'.", write / (1024 * 1024), filename);

  if (args.size() > 4) {
    if (std::stoi(args[4]) != 0) {
      XBT_INFO("Move '%s' (of size %llu) from '%s' to '%s'", filename, file.size(),
               simgrid::s4u::Host::current()->get_cname(), args[2].c_str());
      file.remote_move(simgrid::s4u::Host::by_name(args[2]), args[3]);
    } else {
      XBT_INFO("Copy '%s' (of size %llu) from '%s' to '%s'", filename, file.size(),
               simgrid::s4u::Host::current()->get_cname(), args[2].c_str());
      file.remote_copy(simgrid::s4u::Host::by_name(args[2]), args[3]);
    }
  }
}

int main(int argc, char** argv)
{
  simgrid::s4u::Engine e(&argc, argv);
  sg_storage_file_system_init();
  e.load_platform(argv[1]);
  e.register_function("host", host);
  e.load_deployment(argv[2]);
  std::vector<simgrid::s4u::Host*> all_hosts = e.get_all_hosts();

  for (auto const& h : all_hosts) {
    for (auto const& d : h->get_disks())
      XBT_INFO("Init: %s: %llu/%llu MiB used/free on '%s@%s'", h->get_cname(), sg_disk_get_size_used(d) / INMEGA,
               sg_disk_get_size_free(d) / INMEGA, d->get_cname(), d->get_host()->get_cname());
  }

  e.run();

  for (auto const& h : all_hosts) {
    for (auto const& d : h->get_disks())
      XBT_INFO("End: %llu/%llu MiB used/free on '%s@%s'", sg_disk_get_size_used(d) / INMEGA,
               sg_disk_get_size_free(d) / INMEGA, d->get_cname(), h->get_cname());
  }

  XBT_INFO("Simulation time %g", simgrid::s4u::Engine::get_clock());
  return 0;
}
