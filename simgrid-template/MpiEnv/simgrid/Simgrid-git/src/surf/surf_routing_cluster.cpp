/* Copyright (c) 2009-2011, 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "surf_routing_cluster.hpp"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(surf_route_cluster, surf, "Routing part of surf");

/* This routing is specifically setup to represent clusters, aka homogeneous sets of machines
 * Note that a router is created, easing the interconnexion with the rest of the world.
 */

AS_t model_cluster_create(void)
{
  return new AsCluster();
}

/* Creation routing model functions */
AsCluster::AsCluster() : AsNone()
{
  p_backbone = 0;
  p_loopback = 0;
  p_router = 0;
  p_has_limiter = 0;
  p_has_loopback = 0;
  p_nb_links_per_node = 1;
}

/* Business methods */
void AsCluster::getRouteAndLatency(RoutingEdgePtr src, RoutingEdgePtr dst, sg_platf_route_cbarg_t route, double *lat)
{
  s_surf_parsing_link_up_down_t info;
  XBT_VERB("cluster_get_route_and_latency from '%s'[%d] to '%s'[%d]",
            src->getName(), src->getId(), dst->getName(), dst->getId());

  if (src->getRcType() != SURF_NETWORK_ELEMENT_ROUTER) {    // No specific link for router

    if((src->getId() == dst->getId()) && p_has_loopback  ){
      info = xbt_dynar_get_as(p_linkUpDownList, src->getId() * p_nb_links_per_node, s_surf_parsing_link_up_down_t);
      xbt_dynar_push_as(route->link_list, void *, info.link_up);
      if (lat)
        *lat += static_cast<NetworkLinkPtr>(info.link_up)->getLatency();
      return;
    }


    if (p_has_limiter){          // limiter for sender
      info = xbt_dynar_get_as(p_linkUpDownList, src->getId() * p_nb_links_per_node + p_has_loopback, s_surf_parsing_link_up_down_t);
      xbt_dynar_push_as(route->link_list, void *, info.link_up);
    }

    info = xbt_dynar_get_as(p_linkUpDownList, src->getId() * p_nb_links_per_node + p_has_loopback + p_has_limiter, s_surf_parsing_link_up_down_t);
    if (info.link_up) {         // link up
      xbt_dynar_push_as(route->link_list, void *, info.link_up);
      if (lat)
        *lat += static_cast<NetworkLinkPtr>(info.link_up)->getLatency();
    }

  }

  if (p_backbone) {
    xbt_dynar_push_as(route->link_list, void *, static_cast<ResourcePtr>(p_backbone));
    if (lat)
      *lat += p_backbone->getLatency();
  }

  if (dst->getRcType() != SURF_NETWORK_ELEMENT_ROUTER) {    // No specific link for router
    info = xbt_dynar_get_as(p_linkUpDownList, dst->getId() * p_nb_links_per_node + p_has_loopback + p_has_limiter, s_surf_parsing_link_up_down_t);

    if (info.link_down) {       // link down
      xbt_dynar_push_as(route->link_list, void *, info.link_down);
      if (lat)
        *lat += static_cast<NetworkLinkPtr>(info.link_down)->getLatency();
    }
    if (p_has_limiter){          // limiter for receiver
        info = xbt_dynar_get_as(p_linkUpDownList, dst->getId() * p_nb_links_per_node + p_has_loopback, s_surf_parsing_link_up_down_t);
        xbt_dynar_push_as(route->link_list, void *, info.link_up);
    }
  }
}

void AsCluster::getGraph(xbt_graph_t graph, xbt_dict_t nodes, xbt_dict_t edges)
{
  int isrc;
  int table_size = xbt_dynar_length(p_indexNetworkElm);

  RoutingEdgePtr src;
  xbt_node_t current, previous, backboneNode = NULL, routerNode;
  s_surf_parsing_link_up_down_t info;

  xbt_assert(p_router,"Malformed cluster");

  /* create the router */
  char *link_name = p_router->getName();
  routerNode = new_xbt_graph_node(graph, link_name, nodes);

  if(p_backbone) {
    const char *link_nameR = p_backbone->getName();
    backboneNode = new_xbt_graph_node(graph, link_nameR, nodes);

    new_xbt_graph_edge(graph, routerNode, backboneNode, edges);
  }

  for (isrc = 0; isrc < table_size; isrc++) {
    src = xbt_dynar_get_as(p_indexNetworkElm, isrc, RoutingEdgePtr);

    if (src->getRcType() != SURF_NETWORK_ELEMENT_ROUTER) {
      previous = new_xbt_graph_node(graph, src->getName(), nodes);

      info = xbt_dynar_get_as(p_linkUpDownList, src->getId(), s_surf_parsing_link_up_down_t);

      if (info.link_up) {     // link up

        const char *link_name = static_cast<ResourcePtr>(info.link_up)->getName();
        current = new_xbt_graph_node(graph, link_name, nodes);
        new_xbt_graph_edge(graph, previous, current, edges);

        if (p_backbone) {
          new_xbt_graph_edge(graph, current, backboneNode, edges);
        } else {
          new_xbt_graph_edge(graph, current, routerNode, edges);
        }

      }

      if (info.link_down) {    // link down
        const char *link_name = static_cast<ResourcePtr>(info.link_down)->getName();
        current = new_xbt_graph_node(graph, link_name, nodes);
        new_xbt_graph_edge(graph, previous, current, edges);

        if (p_backbone) {
          new_xbt_graph_edge(graph, current, backboneNode, edges);
        } else {
          new_xbt_graph_edge(graph, current, routerNode, edges);
        }
      }
    }

  }
}

void AsCluster::create_links_for_node(sg_platf_cluster_cbarg_t cluster, int id, int , int position){
  s_sg_platf_link_cbarg_t link;
  s_surf_parsing_link_up_down_t info;
  char* link_id = bprintf("%s_link_%d", cluster->id, id);

  memset(&link, 0, sizeof(link));
  link.id = link_id;
  link.bandwidth = cluster->bw;
  link.latency = cluster->lat;
  link.state = SURF_RESOURCE_ON;
  link.policy = cluster->sharing_policy;
  sg_platf_new_link(&link);

  if (link.policy == SURF_LINK_FULLDUPLEX) {
    char *tmp_link = bprintf("%s_UP", link_id);
    info.link_up = xbt_lib_get_or_null(link_lib, tmp_link, SURF_LINK_LEVEL);
    xbt_free(tmp_link);
    tmp_link = bprintf("%s_DOWN", link_id);
    info.link_down = xbt_lib_get_or_null(link_lib, tmp_link, SURF_LINK_LEVEL);
    xbt_free(tmp_link);
  } else {
    info.link_up = xbt_lib_get_or_null(link_lib, link_id, SURF_LINK_LEVEL);
    info.link_down = info.link_up;
  }
  xbt_dynar_set(p_linkUpDownList, position, &info);
  xbt_free(link_id);
}

int AsCluster::parsePU(RoutingEdgePtr elm) {
  XBT_DEBUG("Load process unit \"%s\"", elm->getName());
  xbt_dynar_push_as(p_indexNetworkElm, RoutingEdgePtr, elm);
  return xbt_dynar_length(p_indexNetworkElm)-1;
}

int AsCluster::parseAS(RoutingEdgePtr elm) {
  XBT_DEBUG("Load Autonomous system \"%s\"", elm->getName());
  xbt_dynar_push_as(p_indexNetworkElm, RoutingEdgePtr, elm);
  return xbt_dynar_length(p_indexNetworkElm)-1;
}

