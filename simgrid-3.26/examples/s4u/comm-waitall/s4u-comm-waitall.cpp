/* Copyright (c) 2010-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/* This example shows how to block on the completion of a set of communications.
 *
 * As for the other asynchronous examples, the sender initiate all the messages it wants to send and
 * pack the resulting simgrid::s4u::CommPtr objects in a vector. All messages thus occur concurrently.
 *
 * The sender then blocks until all ongoing communication terminate, using simgrid::s4u::Comm::wait_all()
 *
 */

#include "simgrid/s4u.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_async_waitall, "Messages specific for this s4u example");

class Sender {
  long messages_count;  /* - number of messages */
  long receivers_count; /* - number of receivers */
  long msg_size;        /* - message size in bytes */

public:
  explicit Sender(std::vector<std::string> args)
  {
    xbt_assert(args.size() == 4, "Expecting 3 parameters from the XML deployment file but got %zu", args.size());
    messages_count  = std::stol(args[1]);
    msg_size        = std::stol(args[2]);
    receivers_count = std::stol(args[3]);
  }
  void operator()() const
  {
    // sphinx-doc: init-begin (this line helps the doc to build; ignore it)
    /* Vector in which we store all ongoing communications */
    std::vector<simgrid::s4u::CommPtr> pending_comms;

    /* Make a vector of the mailboxes to use */
    std::vector<simgrid::s4u::Mailbox*> mboxes;
    for (int i = 0; i < receivers_count; i++)
      mboxes.push_back(simgrid::s4u::Mailbox::by_name(std::string("receiver-") + std::to_string(i)));
    // sphinx-doc: init-end

    /* Start dispatching all messages to receivers, in a round robin fashion */
    for (int i = 0; i < messages_count; i++) {
      std::string msg_content = std::string("Message ") + std::to_string(i);
      // Copy the data we send: the 'msg_content' variable is not a stable storage location.
      // It will be destroyed when this actor leaves the loop, ie before the receiver gets it
      auto* payload = new std::string(msg_content);

      XBT_INFO("Send '%s' to '%s'", msg_content.c_str(), mboxes[i % receivers_count]->get_cname());

      /* Create a communication representing the ongoing communication, and store it in pending_comms */
      simgrid::s4u::CommPtr comm = mboxes[i % receivers_count]->put_async(payload, msg_size);
      pending_comms.push_back(comm);
    }

    /* Start sending messages to let the workers know that they should stop */ // sphinx-doc: put-begin
    for (int i = 0; i < receivers_count; i++) {
      XBT_INFO("Send 'finalize' to 'receiver-%d'", i);
      simgrid::s4u::CommPtr comm = mboxes[i]->put_async(new std::string("finalize"), 0);
      pending_comms.push_back(comm);
    }
    XBT_INFO("Done dispatching all messages");

    /* Now that all message exchanges were initiated, wait for their completion in one single call */
    simgrid::s4u::Comm::wait_all(&pending_comms);
    // sphinx-doc: put-end

    XBT_INFO("Goodbye now!");
  }
};

/* Receiver actor expects 1 argument: its ID */
class Receiver {
  simgrid::s4u::Mailbox* mbox;

public:
  explicit Receiver(std::vector<std::string> args)
  {
    xbt_assert(args.size() == 2, "Expecting one parameter from the XML deployment file but got %zu", args.size());
    std::string mboxName = std::string("receiver-") + args[1];
    mbox                 = simgrid::s4u::Mailbox::by_name(mboxName);
  }
  void operator()()
  {
    XBT_INFO("Wait for my first message");
    for (bool cont = true; cont;) {
      const auto* received = static_cast<std::string*>(mbox->get());
      XBT_INFO("I got a '%s'.", received->c_str());
      cont = (*received != "finalize"); // If it's a finalize message, we're done
      // Receiving the message was all we were supposed to do
      delete received;
    }
  }
};

int main(int argc, char* argv[])
{
  xbt_assert(argc > 2, "Usage: %s platform_file deployment_file\n", argv[0]);

  simgrid::s4u::Engine e(&argc, argv);
  e.register_actor<Sender>("sender");
  e.register_actor<Receiver>("receiver");

  e.load_platform(argv[1]);
  e.load_deployment(argv[2]);
  e.run();

  return 0;
}
