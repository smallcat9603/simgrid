/* Copyright (c) 2010-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/* This code tests that we can change the stack-size between the actors creation. */

#include "simgrid/s4u.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_test, "Messages specific for this s4u example");

static void actor()
{
  XBT_INFO("Hello");
}

int main(int argc, char* argv[])
{
  simgrid::s4u::Engine e(&argc, argv);
  e.load_platform(argv[1]);

  // If you don't specify anything, you get the default size (8Mb) or the one passed on the command line
  simgrid::s4u::Actor::create("actor", simgrid::s4u::Host::by_name("Tremblay"), actor);

  // You can use set_config(string) to pass a size that will be parsed. That value will be used for any subsequent
  // actors
  simgrid::s4u::Engine::set_config("contexts/stack-size:16384");
  simgrid::s4u::Actor::create("actor", simgrid::s4u::Host::by_name("Tremblay"), actor);
  simgrid::s4u::Actor::create("actor", simgrid::s4u::Host::by_name("Tremblay"), actor);

  // You can use set_config(key, value) for the same effect.
  simgrid::s4u::Engine::set_config("contexts/stack-size", 32 * 1024);
  simgrid::s4u::Actor::create("actor", simgrid::s4u::Host::by_name("Tremblay"), actor);
  simgrid::s4u::Actor::create("actor", simgrid::s4u::Host::by_name("Tremblay"), actor);

  // Or you can use set_stacksize() before starting the actor to modify only this one
  simgrid::s4u::Actor::init("actor", simgrid::s4u::Host::by_name("Tremblay"))->set_stacksize(64 * 1024)->start(actor);
  simgrid::s4u::Actor::create("actor", simgrid::s4u::Host::by_name("Tremblay"), actor);

  e.run();
  XBT_INFO("Simulation time %g", e.get_clock());

  return 0;
}
