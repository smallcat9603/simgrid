/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/* This C++ file acts as the foil to the corresponding XML file, where the
   action takes place: Actors are started and stopped at predefined time.   */

#include "simgrid/s4u.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(test, "Messages specific for this s4u example");

/* This actor just sleeps until termination */
class sleeper {

public:
  explicit sleeper(std::vector<std::string> /*args*/)
  {
    simgrid::s4u::this_actor::on_exit(
        [](int, void*) {
          /* Executed on process termination, to display a message helping to understand the output */
          XBT_INFO("Exiting now (done sleeping or got killed).");
        },
        nullptr);

    XBT_INFO("Hello! I go to sleep.");
    simgrid::s4u::this_actor::sleep_for(10);
  }
  void operator()() { XBT_INFO("Done sleeping."); }
};

int main(int argc, char* argv[])
{
  simgrid::s4u::Engine e(&argc, argv);

  xbt_assert(argc > 2, "Usage: %s platform_file deployment_file\n"
                       "\tExample: %s msg_platform.xml msg_deployment.xml\n",
             argv[0], argv[0]);

  e.load_platform(argv[1]); /* Load the platform description */
  e.register_actor<sleeper>("sleeper");
  e.load_deployment(argv[2]); /*  Deploy the sleeper processes with explicit start/kill times */

  e.run(); /* - Run the simulation */

  return 0;
}
