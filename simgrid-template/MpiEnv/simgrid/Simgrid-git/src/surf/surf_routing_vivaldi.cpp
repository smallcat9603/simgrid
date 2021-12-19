/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "surf_routing_vivaldi.hpp"
#include "network_interface.hpp"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(surf_route_vivaldi, surf, "Routing part of surf");

static XBT_INLINE double euclidean_dist_comp(int index, xbt_dynar_t src, xbt_dynar_t dst) {
  double src_coord, dst_coord;

  src_coord = xbt_dynar_get_as(src, index, double);
  dst_coord = xbt_dynar_get_as(dst, index, double);

  return (src_coord-dst_coord)*(src_coord-dst_coord);
}

AS_t model_vivaldi_create(void)
{
  return new AsVivaldi();
}

void AsVivaldi::getRouteAndLatency(RoutingEdgePtr src, RoutingEdgePtr dst, sg_platf_route_cbarg_t route, double *lat)
{
  s_surf_parsing_link_up_down_t info;

  XBT_DEBUG("vivaldi_get_route_and_latency from '%s'[%d] '%s'[%d]",
		  src->getName(), src->getId(), dst->getName(), dst->getId());

  if(src->getRcType() == SURF_NETWORK_ELEMENT_AS) {
    char *src_name = ROUTER_PEER(src->getName());
    char *dst_name = ROUTER_PEER(dst->getName());
    route->gw_src = (sg_routing_edge_t) xbt_lib_get_or_null(as_router_lib, src_name, ROUTING_ASR_LEVEL);
    route->gw_dst = (sg_routing_edge_t) xbt_lib_get_or_null(as_router_lib, dst_name, ROUTING_ASR_LEVEL);
    xbt_free(src_name);
    xbt_free(dst_name);
  }

  double euclidean_dist;
  xbt_dynar_t src_ctn, dst_ctn;
  char *tmp_src_name, *tmp_dst_name;

  if(src->getRcType() == SURF_NETWORK_ELEMENT_HOST){
    tmp_src_name = HOST_PEER(src->getName());

    if(p_linkUpDownList){
      info = xbt_dynar_get_as(p_linkUpDownList, src->getId(), s_surf_parsing_link_up_down_t);
      if(info.link_up) { // link up
        xbt_dynar_push_as(route->link_list, void*, info.link_up);
        if (lat)
          *lat += static_cast<NetworkLinkPtr>(info.link_up)->getLatency();
      }
    }
    src_ctn = (xbt_dynar_t) xbt_lib_get_or_null(host_lib, tmp_src_name, COORD_HOST_LEVEL);
    if(!src_ctn ) src_ctn = (xbt_dynar_t) xbt_lib_get_or_null(host_lib, src->getName(), COORD_HOST_LEVEL);
  }
  else if(src->getRcType() == SURF_NETWORK_ELEMENT_ROUTER || src->getRcType() == SURF_NETWORK_ELEMENT_AS){
    tmp_src_name = ROUTER_PEER(src->getName());
    src_ctn = (xbt_dynar_t) xbt_lib_get_or_null(as_router_lib, tmp_src_name, COORD_ASR_LEVEL);
  }
  else{
    THROW_IMPOSSIBLE;
  }

  if(dst->getRcType() == SURF_NETWORK_ELEMENT_HOST){
    tmp_dst_name = HOST_PEER(dst->getName());

    if(p_linkUpDownList){
      info = xbt_dynar_get_as(p_linkUpDownList, dst->getId(), s_surf_parsing_link_up_down_t);
      if(info.link_down) { // link down
        xbt_dynar_push_as(route->link_list,void*,info.link_down);
        if (lat)
          *lat += static_cast<NetworkLinkPtr>(info.link_down)->getLatency();
      }
    }
    dst_ctn = (xbt_dynar_t) xbt_lib_get_or_null(host_lib, tmp_dst_name, COORD_HOST_LEVEL);
    if(!dst_ctn ) dst_ctn = (xbt_dynar_t) xbt_lib_get_or_null(host_lib, dst->getName(), COORD_HOST_LEVEL);
  }
  else if(dst->getRcType() == SURF_NETWORK_ELEMENT_ROUTER || dst->getRcType() == SURF_NETWORK_ELEMENT_AS){
    tmp_dst_name = ROUTER_PEER(dst->getName());
    dst_ctn = (xbt_dynar_t) xbt_lib_get_or_null(as_router_lib, tmp_dst_name, COORD_ASR_LEVEL);
  }
  else{
    THROW_IMPOSSIBLE;
  }

  xbt_assert(src_ctn,"No coordinate found for element '%s'",tmp_src_name);
  xbt_assert(dst_ctn,"No coordinate found for element '%s'",tmp_dst_name);
  free(tmp_src_name);
  free(tmp_dst_name);

  euclidean_dist = sqrt (euclidean_dist_comp(0,src_ctn,dst_ctn)+euclidean_dist_comp(1,src_ctn,dst_ctn))
                      + fabs(xbt_dynar_get_as(src_ctn, 2, double))+fabs(xbt_dynar_get_as(dst_ctn, 2, double));

  if (lat){
    XBT_DEBUG("Updating latency %f += %f",*lat,euclidean_dist);
    *lat += euclidean_dist / 1000.0; //From .ms to .s
  }
}

int AsVivaldi::parsePU(RoutingEdgePtr elm) {
  XBT_DEBUG("Load process unit \"%s\"", elm->getName());
  xbt_dynar_push_as(p_indexNetworkElm, sg_routing_edge_t, elm);
  return xbt_dynar_length(p_indexNetworkElm)-1;
}
