/* Copyright (c) 2007-2019. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <simgrid/s4u.hpp>

XBT_LOG_NEW_DEFAULT_CATEGORY(sample_simulator, "Messages specific for this simulator");

static void pinger(simgrid::s4u::Mailbox* in, simgrid::s4u::Mailbox* out)
{
  XBT_INFO("Ping from mailbox %s to mailbox %s", in->get_name().c_str(), out->get_name().c_str());
   
  /* - Do the ping with a 1-Byte task (latency bound) ... */
  double* payload = new double(simgrid::s4u::Engine::get_clock());

  out->put(payload, 1);
  /* - ... then wait for the (large) pong */
  double* sender_time = static_cast<double*>(in->get());

  double communication_time = simgrid::s4u::Engine::get_clock() - *sender_time;
  XBT_INFO("Task received : large communication (bandwidth bound)");
  XBT_INFO("Pong time (bandwidth bound): %.3f", communication_time);
  delete sender_time;
}

static void ponger(simgrid::s4u::Mailbox* in, simgrid::s4u::Mailbox* out)
{
  XBT_INFO("Pong from mailbox %s to mailbox %s", in->get_name().c_str(), out->get_name().c_str());

  /* - Receive the (small) ping first ....*/
  double* sender_time = static_cast<double*>(in->get());
  double communication_time = simgrid::s4u::Engine::get_clock() - *sender_time;
  XBT_INFO("Task received : small communication (latency bound)");
  XBT_INFO(" Ping time (latency bound) %f", communication_time);
  delete sender_time;

  /*  - ... Then send a 1GB pong back (bandwidth bound) */
  double* payload = new double();
  *payload        = simgrid::s4u::Engine::get_clock();
  XBT_INFO("task_bw->data = %.3f", *payload);

  out->put(payload, 1e9);
}

int main(int argc, char* argv[])
{
  simgrid::s4u::Engine e(&argc, argv);

  xbt_assert(argc==2, "Usage: %s platform_file.xml", argv[0]);
  e.load_platform(argv[1]);
   
  simgrid::s4u::Mailbox* mb1 = simgrid::s4u::Mailbox::by_name("Mailbox 1");
  simgrid::s4u::Mailbox* mb2 = simgrid::s4u::Mailbox::by_name("Mailbox 2");

  simgrid::s4u::Actor::create("pinger", simgrid::s4u::Host::by_name("node-0.acme.org"), pinger, mb1, mb2);
  simgrid::s4u::Actor::create("ponger", simgrid::s4u::Host::by_name("node-1.acme.org"), ponger, mb2, mb1);

  e.run();

  XBT_INFO("Total simulation time: %.3f", e.get_clock());

  return 0;
}
