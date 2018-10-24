/* Copyright (c) 2009-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/s4u.hpp"
#include "xbt/replay.hpp"
#include "xbt/str.h"
#include <boost/algorithm/string/join.hpp>
#include <string>

XBT_LOG_NEW_DEFAULT_CATEGORY(replay_comm, "Messages specific for this msg example");

#define ACT_DEBUG(...)                                                                                                 \
  if (XBT_LOG_ISENABLED(replay_comm, xbt_log_priority_verbose)) {                                                   \
    std::string NAME = boost::algorithm::join(action, " ");                                                            \
    XBT_DEBUG(__VA_ARGS__);                                                                                            \
  } else                                                                                                               \
  ((void)0)

static void log_action(simgrid::xbt::ReplayAction& action, double date)
{
  if (XBT_LOG_ISENABLED(replay_comm, xbt_log_priority_verbose)) {
    std::string s = boost::algorithm::join(action, " ");
    XBT_VERB("%s %f", s.c_str(), date);
  }
}

class Replayer {
public:
  explicit Replayer(std::vector<std::string> args)
  {
    int argc;
    char* argv[2];
    argv[0] = &args.at(0)[0];
    if (args.size() == 1) {
      argc = 1;
    } else {
      argc    = 2;
      argv[1] = &args.at(1)[0];
    }
    simgrid::xbt::replay_runner(argc, argv);
  }

  void operator()()
  {
    // Nothing to do here
  }

  /* My actions */
  static void compute(simgrid::xbt::ReplayAction& action)
  {
    double amount = std::stod(action[2]);
    double clock  = simgrid::s4u::Engine::get_clock();
    ACT_DEBUG("Entering %s", NAME.c_str());
    simgrid::s4u::this_actor::execute(amount);
    log_action(action, simgrid::s4u::Engine::get_clock() - clock);
  }

  static void send(simgrid::xbt::ReplayAction& action)
  {
    double size                 = std::stod(action[3]);
    std::string* payload        = new std::string(action[3]);
    double clock                = simgrid::s4u::Engine::get_clock();
    simgrid::s4u::MailboxPtr to =
        simgrid::s4u::Mailbox::by_name(simgrid::s4u::this_actor::get_name() + "_" + action[2]);
    ACT_DEBUG("Entering Send: %s (size: %g) -- Actor %s on mailbox %s", NAME.c_str(), size,
              simgrid::s4u::this_actor::get_cname(), to->get_cname());
    to->put(payload, size);
    delete payload;

    log_action(action, simgrid::s4u::Engine::get_clock() - clock);
  }

  static void recv(simgrid::xbt::ReplayAction& action)
  {
    double clock = simgrid::s4u::Engine::get_clock();
    simgrid::s4u::MailboxPtr from =
        simgrid::s4u::Mailbox::by_name(std::string(action[2]) + "_" + simgrid::s4u::this_actor::get_name());

    ACT_DEBUG("Receiving: %s -- Actor %s on mailbox %s", NAME.c_str(), simgrid::s4u::this_actor::get_cname(),
              from->get_cname());
    from->get();
    log_action(action, simgrid::s4u::Engine::get_clock() - clock);
  }
};

int main(int argc, char* argv[])
{
  simgrid::s4u::Engine e(&argc, argv);

  xbt_assert(argc > 2, "Usage: %s platform_file deployment_file [action_files]\n"
                       "\t# if all actions are in the same file\n"
                       "\tExample: %s msg_platform.xml msg_deployment.xml actions\n"
                       "\t# if actions are in separate files, specified in deployment\n"
                       "\tExample: %s msg_platform.xml msg_deployment.xml ",
             argv[0], argv[0], argv[0]);

  e.load_platform(argv[1]);
  e.register_default(&simgrid::xbt::replay_runner);
  e.register_actor<Replayer>("p0");
  e.register_actor<Replayer>("p1");
  e.load_deployment(argv[2]);

  /*   Action registration */
  xbt_replay_action_register("compute", Replayer::compute);
  xbt_replay_action_register("send", Replayer::send);
  xbt_replay_action_register("recv", Replayer::recv);

  if (argv[3]) {
    simgrid::xbt::action_fs = new std::ifstream(argv[3], std::ifstream::in);
  }

  e.run();

  if (argv[3]) {
    delete simgrid::xbt::action_fs;
    simgrid::xbt::action_fs = nullptr;
  }

  XBT_INFO("Simulation time %g", e.get_clock());

  return 0;
}
