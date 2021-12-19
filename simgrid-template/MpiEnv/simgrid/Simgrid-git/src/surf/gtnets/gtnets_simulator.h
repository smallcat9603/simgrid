/* Copyright (c) 2007-2010, 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef _GTNETS_SIM_H
#define _GTNETS_SIM_H

#ifdef __cplusplus
#include "gtnets_topology.h"

#include <iostream>
#include <sys/wait.h>
#include <map>

//GTNetS include files
#include "simulator.h"          // Definitions for the Simulator Object
#include "node.h"               // Definitions for the Node Object
#include "linkp2p.h"            // Definitions for point-to-point link objects
#include "ratetimeparse.h"      // Definitions for Rate and Time objects
#include "application-tcpserver.h"      // Definitions for TCPServer application
#include "application-tcpsend.h"        // Definitions for TCP Sending application
#include "tcp-tahoe.h"          // Definitions for TCP Tahoe
#include "tcp-reno.h"
#include "tcp-newreno.h"
#include "event.h"
#include "routing-manual.h"
#include "red.h"

using namespace std;

//Simulator s;
class GTSim {

public:
  GTSim();
  GTSim(int WindowSize);
  ~GTSim();
public:
  int add_link(int id, double bandwidth, double latency);
  int add_onehop_route(int src, int dst, int link);
  int add_route(int src, int dst, int *links, int nlink);
  int add_router(int id);
  int create_flow(int src, int dst, long datasize, void *metadata);
  double get_time_to_next_flow_completion();
  int run_until_next_flow_completion(void ***metadata,
                                     int *number_of_flows);
  int run(double deltat);
  // returns the total received by the TCPServer peer of the given action
  double gtnets_get_flow_rx(void *metadata);
  void create_gtnets_topology();
  void print_topology();
  void set_jitter(double);
  void set_jitter_seed(int);
private:
  void add_nodes();
  void node_connect();

  bool node_include(int);
  bool link_include(int);
  Simulator *sim_;
  GTNETS_Topology *topo_;
  RoutingManual *rm_;
  REDQueue *red_queue_;
  int nnode_;
  int is_topology_;
  int nflow_;
  double jitter_;
  int jitter_seed_;
   map < int, Uniform * >uniform_jitter_generator_;

   map < int, TCPServer * >gtnets_servers_;
   map < int, TCPSend * >gtnets_clients_;
   map < int, Linkp2p * >gtnets_links_;
   map < int, Node * >gtnets_nodes_;
   map < void *, int >gtnets_action_to_flow_;

   map < int, void *>gtnets_metadata_;
};

#endif                          /* __cplusplus */

#endif
