/* Copyright (c) 2010-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "s4u-dht-chord.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_chord, "Messages specific for this s4u example");
simgrid::xbt::Extension<simgrid::s4u::Host, HostChord> HostChord::EXTENSION_ID;

int nb_bits  = 24;
int nb_keys  = 0;
int timeout  = 50;

int main(int argc, char* argv[])
{
  simgrid::s4u::Engine e(&argc, argv);
  xbt_assert(argc > 2, "Usage: %s [-nb_bits=n] [-timeout=t] platform_file deployment_file\n"
                       "\tExample: %s ../msg_platform.xml chord.xml\n",
             argv[0], argv[0]);
  char** options = &argv[1];
  while (not strncmp(options[0], "-", 1)) {
    unsigned int length = strlen("-nb_bits=");
    if (not strncmp(options[0], "-nb_bits=", length) && strlen(options[0]) > length) {
      nb_bits = xbt_str_parse_int(options[0] + length, "Invalid nb_bits parameter: %s");
      XBT_DEBUG("Set nb_bits to %d", nb_bits);
    } else {
      length = strlen("-timeout=");
      if (not strncmp(options[0], "-timeout=", length) && strlen(options[0]) > length) {
        timeout = xbt_str_parse_int(options[0] + length, "Invalid timeout parameter: %s");
        XBT_DEBUG("Set timeout to %d", timeout);
      } else {
        xbt_die("Invalid chord option '%s'", options[0]);
      }
    }
    options++;
  }

  e.load_platform(options[0]);

  /* Global initialization of the Chord simulation. */
  nb_keys = 1U << nb_bits;
  XBT_DEBUG("Sets nb_keys to %d", nb_keys);

  HostChord::EXTENSION_ID = simgrid::s4u::Host::extension_create<HostChord>();
  for (auto const& host : simgrid::s4u::Engine::get_instance()->get_all_hosts())
    host->extension_set(new HostChord(host));

  e.register_actor<Node>("node");
  e.load_deployment(options[1]);

  e.run();

  XBT_INFO("Simulated time: %g", e.get_clock());
  return 0;
}
