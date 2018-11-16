/* Copyright (c) 2010-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/* This example shows how to use simgrid::s4u::Activity::wait_until() and
 * simgrid::s4u::Activity::wait_for() on a given communication.
 *
 * It is very similar to the async-wait example, but the sender initially
 * does some waits that are too short before doing an infinite wait.
 */

#include "simgrid/s4u.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

XBT_LOG_NEW_DEFAULT_CATEGORY(msg_async_wait, "Messages specific for this s4u example");

static int sender(int argc, char** argv)
{
  xbt_assert(argc == 4, "Expecting 3 parameters from the XML deployment file but got %d", argc);
  long messages_count  = std::stol(argv[1]); /* - number of tasks */
  double msg_size      = std::stol(argv[2]); /* - communication cost in bytes */
  long receivers_count = std::stod(argv[3]); /* - number of receivers */

  std::vector<simgrid::s4u::CommPtr> pending_comms;

  /* Start dispatching all messages to receivers, in a round robin fashion */
  for (int i = 0; i < messages_count; i++) {

    std::string mboxName          = std::string("receiver-") + std::to_string(i % receivers_count);
    simgrid::s4u::MailboxPtr mbox = simgrid::s4u::Mailbox::by_name(mboxName);
    std::string msgName           = std::string("Message ") + std::to_string(i);
    std::string* payload          = new std::string(msgName); // copy the data we send:
    // 'msgName' is not a stable storage location
    XBT_INFO("Send '%s' to '%s'", msgName.c_str(), mboxName.c_str());
    /* Create a communication representing the ongoing communication */
    simgrid::s4u::CommPtr comm = mbox->put_async(payload, msg_size);
    /* Add this comm to the vector of all known comms */
    pending_comms.push_back(comm);
  }

  /* Start sending messages to let the workers know that they should stop */
  for (int i = 0; i < receivers_count; i++) {
    std::string mboxName          = std::string("receiver-") + std::to_string(i % receivers_count);
    simgrid::s4u::MailboxPtr mbox = simgrid::s4u::Mailbox::by_name(mboxName);
    std::string* payload          = new std::string("finalize"); // Make a copy of the data we will send

    simgrid::s4u::CommPtr comm = mbox->put_async(payload, 0);
    pending_comms.push_back(comm);
    XBT_INFO("Send 'finalize' to 'receiver-%ld'", i % receivers_count);
  }
  XBT_INFO("Done dispatching all messages");

  /* Now that all message exchanges were initiated, wait for their completion, in order of creation. */
  while (not pending_comms.empty()) {
    simgrid::s4u::CommPtr comm = pending_comms.back();
    comm->wait_for(1);
    pending_comms.pop_back(); // remove it from the list
  }

  XBT_INFO("Goodbye now!");
  return 0;
}

/* Receiver actor expects 1 argument: its ID */
static int receiver(int argc, char** argv)
{
  xbt_assert(argc == 2, "Expecting one parameter from the XML deployment file but got %d", argc);
  simgrid::s4u::MailboxPtr mbox = simgrid::s4u::Mailbox::by_name(std::string("receiver-") + argv[1]);

  XBT_INFO("Wait for my first message");
  for (bool cont = true; cont;) {
    std::string* received = static_cast<std::string*>(mbox->get());
    XBT_INFO("I got a '%s'.", received->c_str());
    if (*received == "finalize")
      cont = false; // If it's a finalize message, we're done.
    delete received;
  }
  return 0;
}

int main(int argc, char* argv[])
{
  xbt_assert(argc > 2, "Usage: %s platform_file deployment_file\n", argv[0]);

  simgrid::s4u::Engine e(&argc, argv);
  e.register_function("sender", &sender);
  e.register_function("receiver", &receiver);

  e.load_platform(argv[1]);
  e.load_deployment(argv[2]);
  e.run();

  return 0;
}
