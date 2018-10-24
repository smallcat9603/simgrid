/* Copyright (c) 2013-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SURF_ROUTING_FLOYD_HPP_
#define SURF_ROUTING_FLOYD_HPP_

#include <simgrid/kernel/routing/RoutedZone.hpp>

namespace simgrid {
namespace kernel {
namespace routing {

/** @ingroup ROUTING_API
 *  @brief NetZone with an explicit routing computed at initialization with Floyd-Warshal
 *
 *  The path between components is computed at creation time from every one-hop links,
 *  using the Floyd-Warshal algorithm.
 *
 *  This result in rather small platform file, slow initialization time,  and intermediate memory requirements
 *  (somewhere between the one of @{DijkstraZone} and the one of @{FullZone}).
 */
class XBT_PRIVATE FloydZone : public RoutedZone {
public:
  explicit FloydZone(NetZoneImpl* father, std::string name, resource::NetworkModel* netmodel);
  ~FloydZone() override;

  void get_local_route(NetPoint* src, NetPoint* dst, RouteCreationArgs* into, double* latency) override;
  void add_route(NetPoint* src, NetPoint* dst, NetPoint* gw_src, NetPoint* gw_dst,
                 std::vector<resource::LinkImpl*>& link_list, bool symmetrical) override;
  void seal() override;

private:
  /* vars to compute the Floyd algorithm. */
  int* predecessor_table_;
  double* cost_table_;
  RouteCreationArgs** link_table_;
};
} // namespace routing
} // namespace kernel
} // namespace simgrid

#endif /* SURF_ROUTING_FLOYD_HPP_ */
