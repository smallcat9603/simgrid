/* Copyright (c) 2017-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/s4u.hpp"

/* This example does not much: It just spans over-polite actor that yield a large amount
 * of time before ending.
 *
 * This serves as an example for the simgrid::s4u::this_actor::yield() function, with which an actor can request
 * to be rescheduled after the other actor that are ready at the current timestamp.
 *
 * It can also be used to benchmark our context-switching mechanism.
 */
XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_actor_yield, "Messages specific for this s4u example");
/* Main function of the Yielder process */
class yielder {
  long number_of_yields;

public:
  explicit yielder(std::vector<std::string> args) { number_of_yields = std::stod(args[1]); }
  void operator()()
  {
    for (int i = 0; i < number_of_yields; i++)
      simgrid::s4u::this_actor::yield();
    XBT_INFO("I yielded %ld times. Goodbye now!", number_of_yields);
  }
};

int main(int argc, char* argv[])
{
  simgrid::s4u::Engine e(&argc, argv);

  xbt_assert(argc > 2, "Usage: %s platform_file deployment_file\n"
                       "\tExample: %s platform.xml deployment.xml\n",
             argv[0], argv[0]);

  e.load_platform(argv[1]);             /* Load the platform description */
  e.register_actor<yielder>("yielder"); /* Register the class representing the actors */

  e.load_deployment(argv[2]);

  e.run(); /* - Run the simulation */

  return 0;
}
