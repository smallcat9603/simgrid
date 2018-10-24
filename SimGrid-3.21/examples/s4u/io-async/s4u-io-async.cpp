/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/s4u.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_test, "Messages specific for this s4u example");

static void test(sg_size_t size)
{
  simgrid::s4u::Storage* storage = simgrid::s4u::Storage::by_name("Disk1");
  XBT_INFO("Hello! read %llu bytes from Storage %s", size, storage->get_cname());

  simgrid::s4u::IoPtr activity = storage->io_init(size, simgrid::s4u::Io::OpType::READ);
  activity->start();
  activity->wait();

  XBT_INFO("Goodbye now!");
}

static void test_cancel(sg_size_t size)
{
  simgrid::s4u::Storage* storage = simgrid::s4u::Storage::by_name("Disk2");
  XBT_INFO("Hello! write %llu bytes from Storage %s", size, storage->get_cname());

  simgrid::s4u::IoPtr activity = storage->write_async(size);
  simgrid::s4u::this_actor::sleep_for(0.5);
  XBT_INFO("I changed my mind, cancel!");
  activity->cancel();

  XBT_INFO("Goodbye now!");
}

int main(int argc, char* argv[])
{
  simgrid::s4u::Engine e(&argc, argv);
  e.load_platform(argv[1]);
  simgrid::s4u::Actor::create("test", simgrid::s4u::Host::by_name("bob"), test, 2e7);
  simgrid::s4u::Actor::create("test_cancel", simgrid::s4u::Host::by_name("alice"), test_cancel, 5e7);

  e.run();

  XBT_INFO("Simulation time %g", e.get_clock());

  return 0;
}
