/* Copyright (c) 2012-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "s4u-dht-kademlia.hpp"

#include "message.hpp"
#include "node.hpp"
#include "simgrid/s4u.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(kademlia, "Messages specific for this example");

/** @brief Node function
  * @param my node ID
  * @param the ID of the person I know in the system (or not)
  * @param Time before I leave the system because I'm bored
  */
static int node(int argc, char* argv[])
{
  bool join_success = true;
  double deadline;
  xbt_assert(argc == 3 || argc == 4, "Wrong number of arguments");
  /* Node initialization */
  unsigned int node_id = strtoul(argv[1], nullptr, 0);
  kademlia::Node* node = new kademlia::Node(node_id);

  if (argc == 4) {
    XBT_INFO("Hi, I'm going to join the network with id %u", node->getId());
    unsigned int known_id = strtoul(argv[2], NULL, 0);
    join_success          = node->join(known_id);
    deadline              = std::stod(argv[3]);
  } else {
    deadline = std::stod(argv[2]);
    XBT_INFO("Hi, I'm going to create the network with id %u", node->getId());
    node->routingTableUpdate(node->getId());
  }

  if (join_success) {
    XBT_VERB("Ok, I'm joining the network with id %u", node->getId());
    // We start the main loop
    double next_lookup_time = simgrid::s4u::Engine::get_clock() + random_lookup_interval;

    XBT_VERB("Main loop start");

    simgrid::s4u::MailboxPtr mailbox = simgrid::s4u::Mailbox::by_name(std::to_string(node->getId()));

    while (simgrid::s4u::Engine::get_clock() < deadline) {
      if (node->receive_comm == nullptr)
        node->receive_comm = mailbox->get_async(&node->received_msg);

      if (node->receive_comm->test()) {
        // There has been a message, we need to handle it !
        kademlia::Message* msg = static_cast<kademlia::Message*>(node->received_msg);
        if (msg) {
          node->handleFindNode(msg);
          delete msg->answer_;
          delete msg;
          node->receive_comm = nullptr;
        } else
          simgrid::s4u::this_actor::sleep_for(1);
      } else {
        /* We search for a pseudo random node */
        if (simgrid::s4u::Engine::get_clock() >= next_lookup_time) {
          node->randomLookup();
          next_lookup_time += random_lookup_interval;
        } else {
          // Didn't get a message: sleep for a while...
          simgrid::s4u::this_actor::sleep_for(1);
        }
      }
    }
  } else {
    XBT_INFO("I couldn't join the network :(");
  }
  XBT_DEBUG("I'm leaving the network");
  XBT_INFO("%u/%u FIND_NODE have succeeded", node->find_node_success, node->find_node_success + node->find_node_failed);
  delete node;

  return 0;
}

/** @brief Main function */
int main(int argc, char* argv[])
{
  simgrid::s4u::Engine e(&argc, argv);

  /* Check the arguments */
  xbt_assert(argc > 2,
             "Usage: %s platform_file deployment_file\n\tExample: %s cluster_backbone.xml dht-kademlia_d.xml\n",
             argv[0], argv[0]);

  e.load_platform(argv[1]);
  e.register_function("node", node);
  e.load_deployment(argv[2]);

  e.run();

  XBT_INFO("Simulated time: %g", simgrid::s4u::Engine::get_clock());

  return 0;
}
