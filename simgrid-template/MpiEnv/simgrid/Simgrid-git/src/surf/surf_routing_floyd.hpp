/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "surf_routing_generic.hpp"

#ifndef SURF_ROUTING_FLOYD_HPP_
#define SURF_ROUTING_FLOYD_HPP_


/***********
 * Classes *
 ***********/
class AsFloyd;
typedef AsFloyd *AsFloydPtr;

class AsFloyd: public AsGeneric {
public:
  AsFloyd();
  ~AsFloyd();

  void getRouteAndLatency(RoutingEdgePtr src, RoutingEdgePtr dst, sg_platf_route_cbarg_t into, double *latency);
  xbt_dynar_t getOneLinkRoutes();
  void parseASroute(sg_platf_route_cbarg_t route);
  void parseRoute(sg_platf_route_cbarg_t route);
  void end();
  //void parseASroute(sg_platf_route_cbarg_t route);

  //void getGraph(xbt_graph_t graph, xbt_dict_t nodes, xbt_dict_t edges);
  //sg_platf_route_cbarg_t getBypassRoute(RoutingEdgePtr src, RoutingEdgePtr dst, double *lat);

  /* The parser calls the following functions to inform the routing models
   * that a new element is added to the AS currently built.
   *
   * Of course, only the routing model of this AS is informed, not every ones */
  //int parsePU(RoutingEdgePtr elm); /* A host or a router, whatever */
  //int parseAS(RoutingEdgePtr elm);

  //virtual void parseBypassroute(sg_platf_route_cbarg_t e_route)=0;

  /* vars for calculate the floyd algorith. */
  int *p_predecessorTable;
  double *p_costTable;
  sg_platf_route_cbarg_t *p_linkTable;
};



#endif /* SURF_ROUTING_FLOYD_HPP_ */
