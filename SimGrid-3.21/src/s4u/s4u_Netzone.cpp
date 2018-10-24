/* Copyright (c) 2006-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/kernel/routing/NetPoint.hpp"
#include "simgrid/s4u/Engine.hpp"
#include "simgrid/s4u/Host.hpp"
#include "simgrid/s4u/NetZone.hpp"
#include "simgrid/zone.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_netzone, "S4U Networking Zones");

namespace simgrid {
namespace s4u {

simgrid::xbt::signal<void(bool symmetrical, kernel::routing::NetPoint* src, kernel::routing::NetPoint* dst,
                          kernel::routing::NetPoint* gw_src, kernel::routing::NetPoint* gw_dst,
                          std::vector<kernel::resource::LinkImpl*>& link_list)>
    NetZone::on_route_creation;
simgrid::xbt::signal<void(NetZone&)> NetZone::on_creation;
simgrid::xbt::signal<void(NetZone&)> NetZone::on_seal;

NetZone::NetZone(kernel::routing::NetZoneImpl* impl) : pimpl_(impl) {}

NetZone::~NetZone()
{
}

std::unordered_map<std::string, std::string>* NetZone::get_properties()
{
  return simgrid::simix::simcall([this] { return &properties_; });
}

/** Retrieve the property value (or nullptr if not set) */
const char* NetZone::get_property(std::string key)
{
  return properties_.at(key).c_str();
}
void NetZone::set_property(std::string key, std::string value)
{
  simgrid::simix::simcall([this, key, value] { properties_[key] = value; });
}

/** @brief Returns the list of direct children (no grand-children) */
std::vector<NetZone*> NetZone::get_children()
{
  std::vector<NetZone*> res;
  for (auto child : *(pimpl_->get_children()))
    res.push_back(child->get_iface());
  return res;
}

const std::string& NetZone::get_name() const
{
  return pimpl_->get_name();
}
const char* NetZone::get_cname() const
{
  return pimpl_->get_cname();
}
NetZone* NetZone::get_father()
{
  return pimpl_->get_father()->get_iface();
}

/** @brief Returns the list of the hosts found in this NetZone (not recursively)
 *
 * Only the hosts that are directly contained in this NetZone are retrieved,
 * not the ones contained in sub-netzones.
 */
std::vector<Host*> NetZone::get_all_hosts()
{
  return pimpl_->get_all_hosts();
}

void NetZone::getHosts(std::vector<s4u::Host*>* whereto)
{
  for (auto const& card : pimpl_->get_vertices()) {
    s4u::Host* host = simgrid::s4u::Host::by_name_or_null(card->get_name());
    if (host != nullptr)
      whereto->push_back(host);
  }
}

int NetZone::get_host_count()
{
  return pimpl_->get_host_count();
}

int NetZone::add_component(kernel::routing::NetPoint* elm)
{
  return pimpl_->add_component(elm);
}

void NetZone::add_route(kernel::routing::NetPoint* src, kernel::routing::NetPoint* dst,
                        kernel::routing::NetPoint* gw_src, kernel::routing::NetPoint* gw_dst,
                        std::vector<kernel::resource::LinkImpl*>& link_list, bool symmetrical)
{
  pimpl_->add_route(src, dst, gw_src, gw_dst, link_list, symmetrical);
}
void NetZone::add_bypass_route(kernel::routing::NetPoint* src, kernel::routing::NetPoint* dst,
                               kernel::routing::NetPoint* gw_src, kernel::routing::NetPoint* gw_dst,
                               std::vector<kernel::resource::LinkImpl*>& link_list, bool symmetrical)
{
  pimpl_->add_bypass_route(src, dst, gw_src, gw_dst, link_list, symmetrical);
}
std::vector<kernel::routing::NetPoint*> NetZone::getVertices()
{
  return pimpl_->get_vertices();
}
} // namespace s4u
} // namespace simgrid

/* **************************** Public C interface *************************** */

sg_netzone_t sg_zone_get_root()
{
  return simgrid::s4u::Engine::get_instance()->get_netzone_root();
}

const char* sg_zone_get_name(sg_netzone_t netzone)
{
  return netzone->get_cname();
}

sg_netzone_t sg_zone_get_by_name(const char* name)
{
  return simgrid::s4u::Engine::get_instance()->netzone_by_name_or_null(name);
}

void sg_zone_get_sons(sg_netzone_t netzone, xbt_dict_t whereto)
{
  for (auto const& elem : netzone->get_children()) {
    xbt_dict_set(whereto, elem->get_cname(), static_cast<void*>(elem), nullptr);
  }
}

const char* sg_zone_get_property_value(sg_netzone_t netzone, const char* name)
{
  return netzone->get_property(name);
}

void sg_zone_set_property_value(sg_netzone_t netzone, const char* name, char* value)
{
  netzone->set_property(name, value);
}

void sg_zone_get_hosts(sg_netzone_t netzone, xbt_dynar_t whereto)
{
  /* converts vector to dynar */
  std::vector<simgrid::s4u::Host*> hosts = netzone->get_all_hosts();
  for (auto const& host : hosts)
    xbt_dynar_push(whereto, &host);
}
