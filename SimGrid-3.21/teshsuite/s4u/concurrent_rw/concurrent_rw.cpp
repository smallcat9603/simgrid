/* Copyright (c) 2008-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/s4u.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_test, "Messages specific for this s4u test");

static void host()
{
  simgrid::s4u::Storage* storage = simgrid::s4u::Storage::by_name("Disk1");
  int id                         = simgrid::s4u::this_actor::get_pid();
  XBT_INFO("process %d is writing!", id);
  storage->write(3000000);
  XBT_INFO("process %d goes to sleep for %d seconds", id, id);
  simgrid::s4u::this_actor::sleep_for(id);
  XBT_INFO("process %d is writing again!", id);
  storage->write(3000000);
  XBT_INFO("process %d goes to sleep for %d seconds", id, 6 - id);
  simgrid::s4u::this_actor::sleep_for(6 - id);
  XBT_INFO("process %d is reading!", id);
  storage->read(3000000);
  XBT_INFO("process %d goes to sleep for %d seconds", id, id);
  simgrid::s4u::this_actor::sleep_for(id);
  XBT_INFO("process %d is reading again!", id);
  storage->read(3000000);
}

int main(int argc, char** argv)
{
  simgrid::s4u::Engine e(&argc, argv);
  e.load_platform(argv[1]);

  for (int i = 0; i < 5; i++)
    simgrid::s4u::Actor::create("host", simgrid::s4u::Host::by_name("bob"), host);

  e.run();
  XBT_INFO("Simulation time %g", e.get_clock());

  return 0;
}
