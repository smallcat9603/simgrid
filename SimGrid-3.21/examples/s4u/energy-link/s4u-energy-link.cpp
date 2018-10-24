/* Copyright (c) 2017-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/plugins/energy.h"
#include "xbt/log.h"
#include <simgrid/s4u.hpp>

#include <random>

/* Parameters of the random generation of the flow size */
static const unsigned long int min_size = 1e6;
static const unsigned long int max_size = 1e9;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_app_energyconsumption, "Messages specific for this s4u example");

static void sender(std::vector<std::string> args)
{
  xbt_assert(args.size() == 2, "The master function expects 2 arguments.");
  int flow_amount  = std::stoi(args.at(0));
  double comm_size = std::stod(args.at(1));
  XBT_INFO("Send %.0f bytes, in %d flows", comm_size, flow_amount);

  simgrid::s4u::MailboxPtr mailbox = simgrid::s4u::Mailbox::by_name(std::string("message"));

  /* Sleep a while before starting the example */
  simgrid::s4u::this_actor::sleep_for(10);


  if (flow_amount == 1) {
    /* - Send the task to the @ref worker */
    char* payload = bprintf("%f", comm_size);
    mailbox->put(payload, comm_size);
  } else {
    // Start all comms in parallel, and wait for all completions in one shot
    std::vector<simgrid::s4u::CommPtr> comms;
    for (int i = 0; i < flow_amount; i++)
      comms.push_back(mailbox->put_async(bprintf("%d", i), comm_size));
    simgrid::s4u::Comm::wait_all(&comms);
  }
  XBT_INFO("sender done.");
}

static void receiver(std::vector<std::string> args)
{
  int flow_amount = std::stoi(args.at(0));

  XBT_INFO("Receiving %d flows ...", flow_amount);

  simgrid::s4u::MailboxPtr mailbox = simgrid::s4u::Mailbox::by_name(std::string("message"));

  if (flow_amount == 1) {
    void* res = mailbox->get();
    xbt_free(res);
  } else {
    void* data[flow_amount];

    // Start all comms in parallel, and wait for their completion in one shot
    std::vector<simgrid::s4u::CommPtr> comms;
    for (int i = 0; i < flow_amount; i++)
      comms.push_back(mailbox->get_async(&data[i]));

    simgrid::s4u::Comm::wait_all(&comms);
    for (int i = 0; i < flow_amount; i++)
      xbt_free(data[i]);
  }
  XBT_INFO("receiver done.");
}

int main(int argc, char* argv[])
{

  simgrid::s4u::Engine e(&argc, argv);

  XBT_INFO("Activating the SimGrid link energy plugin");
  sg_link_energy_plugin_init();

  xbt_assert(argc > 1, "\nUsage: %s platform_file [flowCount [datasize]]\n"
                       "\tExample: %s s4uplatform.xml \n",
             argv[0], argv[0]);
  e.load_platform(argv[1]);

  /* prepare to launch the actors */
  std::vector<std::string> argSender;
  std::vector<std::string> argReceiver;
  if (argc > 2) {
    argSender.push_back(argv[2]); // Take the amount of flows from the command line
    argReceiver.push_back(argv[2]);
  } else {
    argSender.push_back("1"); // Default value
    argReceiver.push_back("1");
  }
  if (argc > 3) {
    if (strcmp(argv[3], "random") == 0) { // We're asked to get a random size
      /* Initialize the random number generator */
      std::random_device rd;
      std::default_random_engine generator(rd());

      /* Distribution on which to apply the generator */
      std::uniform_int_distribution<unsigned long int> distribution(min_size, max_size);

      char* size = bprintf("%lu", distribution(generator));
      argSender.push_back(std::string(size));
      xbt_free(size);
    } else {                        // Not "random" ? Then it should be the size to use
      argSender.push_back(argv[3]); // Take the datasize from the command line
    }
  } else { // No parameter at all? Then use the default value
    argSender.push_back("25000");
  }
  simgrid::s4u::Actor::create("sender", simgrid::s4u::Host::by_name("MyHost1"), sender, argSender);
  simgrid::s4u::Actor::create("receiver", simgrid::s4u::Host::by_name("MyHost2"), receiver, argReceiver);

  /* And now, launch the simulation */
  e.run();

  return 0;
}
