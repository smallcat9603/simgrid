/* platf_private.h - Interface to the SimGrid platforms which visibility should be limited to this directory */

/* Copyright (c) 2004-2020. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SG_PLATF_H
#define SG_PLATF_H

#include "simgrid/host.h"
#include "simgrid/s4u/Link.hpp"
#include "src/surf/xml/platf.hpp"
#include "src/surf/xml/simgrid_dtd.h"

#include <map>
#include <string>
#include <vector>

namespace simgrid {
namespace kernel {
namespace routing {
/* ***************************************** */
/*
 * Platform creation functions. Instead of passing 123 arguments to the creation functions
 * (one for each possible XML attribute), we pass structures containing them all. It removes the
 * chances of switching arguments by error, and reduce the burden when we add a new attribute:
 * old models can just continue to ignore it without having to update their headers.
 *
 * It shouldn't be too costly at runtime, provided that structures living on the stack are
 * used, instead of malloced structures.
 */

class HostCreationArgs {
public:
  std::string id;
  std::vector<double> speed_per_pstate;
  int pstate               = 0;
  int core_amount          = 0;
  profile::Profile* speed_trace                            = nullptr;
  profile::Profile* state_trace                            = nullptr;
  std::string coord                                        = "";
  std::unordered_map<std::string, std::string>* properties = nullptr;
  std::vector<simgrid::kernel::resource::DiskImpl*> disks;
};

class HostLinkCreationArgs {
public:
  std::string id;
  std::string link_up;
  std::string link_down;
};

class LinkCreationArgs {
public:
  std::string id;
  std::vector<double> bandwidths;
  profile::Profile* bandwidth_trace                        = nullptr;
  double latency                      = 0;
  profile::Profile* latency_trace                          = nullptr;
  profile::Profile* state_trace                            = nullptr;
  simgrid::s4u::Link::SharingPolicy policy       = simgrid::s4u::Link::SharingPolicy::FATPIPE;
  std::unordered_map<std::string, std::string>* properties = nullptr;
};

class PeerCreationArgs {
public:
  std::string id;
  double speed;
  double bw_in;
  double bw_out;
  std::string coord;
  profile::Profile* speed_trace;
  profile::Profile* state_trace;
};

class RouteCreationArgs {
public:
  bool symmetrical = false;
  NetPoint* src    = nullptr;
  NetPoint* dst    = nullptr;
  NetPoint* gw_src = nullptr;
  NetPoint* gw_dst = nullptr;
  std::vector<simgrid::kernel::resource::LinkImpl*> link_list;
};

enum class ClusterTopology { DRAGONFLY = 3, FAT_TREE = 2, FLAT = 1, TORUS = 0 };

class ClusterCreationArgs {
public:
  std::string id;
  std::string prefix;
  std::string suffix;
  std::vector<int>* radicals = nullptr;
  std::vector<double> speeds;
  int core_amount     = 0;
  double bw           = 0;
  double lat          = 0;
  double bb_bw        = 0;
  double bb_lat       = 0;
  double loopback_bw  = 0;
  double loopback_lat = 0;
  double limiter_link = 0;
  ClusterTopology topology = ClusterTopology::FLAT;
  std::string topo_parameters;
  std::unordered_map<std::string, std::string>* properties = nullptr;
  std::string router_id;
  simgrid::s4u::Link::SharingPolicy sharing_policy    = simgrid::s4u::Link::SharingPolicy::SPLITDUPLEX;
  simgrid::s4u::Link::SharingPolicy bb_sharing_policy = simgrid::s4u::Link::SharingPolicy::SHARED;
};

class CabinetCreationArgs {
public:
  std::string id;
  std::string prefix;
  std::string suffix;
  std::vector<int>* radicals;
  double speed;
  double bw;
  double lat;
};

class StorageCreationArgs {
public:
  std::string filename;
  int lineno;
  std::string id;
  std::string type_id;
  std::string content;
  std::unordered_map<std::string, std::string>* properties;
  std::string attach;
};

class StorageTypeCreationArgs {
public:
  std::string id;
  std::string model;
  std::string content;
  std::unordered_map<std::string, std::string>* properties;
  std::unordered_map<std::string, std::string>* model_properties;
  sg_size_t size;
};

class DiskCreationArgs {
public:
  std::string id;
  std::unordered_map<std::string, std::string>* properties;
  double read_bw;
  double write_bw;
};

class MountCreationArgs {
public:
  std::string storageId;
  std::string name;
};

class ProfileCreationArgs {
public:
  std::string id;
  std::string file;
  double periodicity;
  std::string pc_data;
};

enum class TraceConnectKind { HOST_AVAIL, SPEED, LINK_AVAIL, BANDWIDTH, LATENCY };

class TraceConnectCreationArgs {
public:
  TraceConnectKind kind;
  std::string trace;
  std::string element;
};

class ActorCreationArgs {
public:
  std::vector<std::string> args;
  std::unordered_map<std::string, std::string>* properties = nullptr;
  const char* host                       = nullptr;
  const char* function                   = nullptr;
  double start_time                      = 0.0;
  double kill_time                       = 0.0;
  bool restart_on_failure                                  = false;
};

class ZoneCreationArgs {
public:
  std::string id;
  std::string routing;
};

extern XBT_PRIVATE xbt::signal<void(ClusterCreationArgs const&)> on_cluster_creation;

} // namespace routing
} // namespace kernel
} // namespace simgrid

/********** Routing **********/
void routing_cluster_add_backbone(simgrid::kernel::resource::LinkImpl* bb);
/*** END of the parsing cruft ***/

XBT_PUBLIC simgrid::kernel::routing::NetZoneImpl*
sg_platf_new_Zone_begin(const simgrid::kernel::routing::ZoneCreationArgs* zone); // Begin description of new Zone
XBT_PUBLIC void sg_platf_new_Zone_set_properties(const std::unordered_map<std::string, std::string>* props);
XBT_PUBLIC void sg_platf_new_Zone_seal();                                          // That Zone is fully described

XBT_PUBLIC void
sg_platf_new_host(const simgrid::kernel::routing::HostCreationArgs* host); // Add a host to the current Zone
XBT_PUBLIC void
sg_platf_new_hostlink(const simgrid::kernel::routing::HostLinkCreationArgs* h); // Add a host_link to the current Zone
XBT_PUBLIC void
sg_platf_new_link(const simgrid::kernel::routing::LinkCreationArgs* link); // Add a link to the current Zone
XBT_PUBLIC void
sg_platf_new_peer(const simgrid::kernel::routing::PeerCreationArgs* peer); // Add a peer to the current Zone
XBT_PUBLIC void sg_platf_new_cluster(simgrid::kernel::routing::ClusterCreationArgs* clust);   // Add a cluster   to the current Zone
XBT_PUBLIC void
sg_platf_new_cabinet(const simgrid::kernel::routing::CabinetCreationArgs* cabinet); // Add a cabinet to the current Zone
XBT_PUBLIC simgrid::kernel::routing::NetPoint* // Add a router    to the current Zone
    sg_platf_new_router(const std::string&, const char* coords);

XBT_PUBLIC void sg_platf_new_route(simgrid::kernel::routing::RouteCreationArgs* route);             // Add a route
XBT_PUBLIC void sg_platf_new_bypassRoute(simgrid::kernel::routing::RouteCreationArgs* bypassroute); // Add a bypassRoute

XBT_PUBLIC void sg_platf_new_trace(simgrid::kernel::routing::ProfileCreationArgs* trace);

XBT_PUBLIC simgrid::kernel::resource::DiskImpl*
sg_platf_new_disk(const simgrid::kernel::routing::DiskCreationArgs* disk); // Add a disk to the current host

XBT_PUBLIC void sg_platf_new_storage(simgrid::kernel::routing::StorageCreationArgs* storage); // Add a storage to the current Zone
XBT_PUBLIC void sg_platf_new_storage_type(const simgrid::kernel::routing::StorageTypeCreationArgs* storage_type);
XBT_PUBLIC void sg_platf_new_mount(simgrid::kernel::routing::MountCreationArgs* mount);

XBT_PUBLIC void sg_platf_new_actor(simgrid::kernel::routing::ActorCreationArgs* actor);
XBT_PRIVATE void sg_platf_trace_connect(simgrid::kernel::routing::TraceConnectCreationArgs* trace_connect);

/* Prototypes of the functions offered by flex */
XBT_PUBLIC int surf_parse_lex();
XBT_PUBLIC int surf_parse_get_lineno();
XBT_PUBLIC FILE* surf_parse_get_in();
XBT_PUBLIC FILE* surf_parse_get_out();
XBT_PUBLIC int surf_parse_get_leng();
XBT_PUBLIC char* surf_parse_get_text();
XBT_PUBLIC void surf_parse_set_lineno(int line_number);
XBT_PUBLIC void surf_parse_set_in(FILE* in_str);
XBT_PUBLIC void surf_parse_set_out(FILE* out_str);
XBT_PUBLIC int surf_parse_get_debug();
XBT_PUBLIC void surf_parse_set_debug(int bdebug);
XBT_PUBLIC int surf_parse_lex_destroy();

#endif                          /* SG_PLATF_H */
