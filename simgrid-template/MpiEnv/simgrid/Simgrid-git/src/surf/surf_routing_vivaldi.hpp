/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "surf_routing_generic.hpp"

#ifndef SURF_ROUTING_VIVALDI_HPP_
#define SURF_ROUTING_VIVALDI_HPP_

/* ************************************************** */
/* **************  Vivaldi ROUTING   **************** */
AS_t model_vivaldi_create(void);      /* create structures for vivaldi routing model */
#define HOST_PEER(peername) bprintf("peer_%s", peername)
#define ROUTER_PEER(peername) bprintf("router_%s", peername)
#define LINK_PEER(peername) bprintf("link_%s", peername)

/***********
 * Classes *
 ***********/
class AsVivaldi;
typedef AsVivaldi *AsVivaldiPtr;

class AsVivaldi: public AsGeneric {
public:
  sg_platf_route_cbarg_t *p_routingTable;

  AsVivaldi() : AsGeneric() {};
  ~AsVivaldi() {};

  void getRouteAndLatency(RoutingEdgePtr src, RoutingEdgePtr dst, sg_platf_route_cbarg_t into, double *latency);
  //void getGraph(xbt_graph_t graph, xbt_dict_t nodes, xbt_dict_t edges);
  //sg_platf_route_cbarg_t getBypassRoute(RoutingEdgePtr src, RoutingEdgePtr dst, double *lat);

  /* The parser calls the following functions to inform the routing models
   * that a new element is added to the AS currently built.
   *
   * Of course, only the routing model of this AS is informed, not every ones */
  int parsePU(RoutingEdgePtr elm); /* A host or a router, whatever */
  //virtual int parseAS( RoutingEdgePtr elm)=0;

  //virtual void parseBypassroute(sg_platf_route_cbarg_t e_route)=0;
};


#endif /* SURF_ROUTING_VIVALDI_HPP_ */
