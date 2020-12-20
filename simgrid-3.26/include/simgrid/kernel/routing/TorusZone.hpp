/* Copyright (c) 2014-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SURF_ROUTING_CLUSTER_TORUS_HPP_
#define SURF_ROUTING_CLUSTER_TORUS_HPP_

#include <simgrid/kernel/routing/ClusterZone.hpp>

#include <vector>

namespace simgrid {
namespace kernel {
namespace routing {

/** @ingroup ROUTING_API
 * @brief NetZone using a Torus topology
 *
 */

class XBT_PRIVATE TorusZone : public ClusterZone {
public:
  explicit TorusZone(NetZoneImpl* father, const std::string& name, resource::NetworkModel* netmodel);
  void create_links_for_node(ClusterCreationArgs* cluster, int id, int rank, unsigned int position) override;
  void get_local_route(NetPoint* src, NetPoint* dst, RouteCreationArgs* into, double* latency) override;
  void parse_specific_arguments(ClusterCreationArgs* cluster) override;

private:
  std::vector<unsigned int> dimensions_;
};
} // namespace routing
} // namespace kernel
} // namespace simgrid
#endif
