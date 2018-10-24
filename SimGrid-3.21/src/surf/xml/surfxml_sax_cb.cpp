/* Copyright (c) 2006-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/kernel/routing/NetPoint.hpp"
#include "simgrid/s4u/Engine.hpp"
#include "simgrid/sg_config.hpp"
#include "src/surf/network_interface.hpp"
#include "src/surf/surf_interface.hpp"
#include "src/surf/xml/platf_private.hpp"
#include "surf/surf.hpp"
#include "xbt/file.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(surf_parse, surf, "Logging specific to the SURF parsing module");

#include "simgrid_dtd.c"

static std::string surf_parsed_filename; // Currently parsed file (for the error messages)
std::vector<simgrid::kernel::resource::LinkImpl*>
    parsed_link_list; /* temporary store of current list link of a route */

/*
 * Helping functions
 */
void surf_parse_assert(bool cond, std::string msg)
{
  if (not cond) {
    int lineno = surf_parse_lineno;
    cleanup();
    XBT_ERROR("Parse error at %s:%d: %s", surf_parsed_filename.c_str(), lineno, msg.c_str());
    surf_exit();
    xbt_die("Exiting now");
  }
}

void surf_parse_error(std::string msg)
{
  int lineno = surf_parse_lineno;
  cleanup();
  XBT_ERROR("Parse error at %s:%d: %s", surf_parsed_filename.c_str(), lineno, msg.c_str());
  surf_exit();
  xbt_die("Exiting now");
}

void surf_parse_assert_netpoint(std::string hostname, std::string pre, std::string post)
{
  if (sg_netpoint_by_name_or_null(hostname.c_str()) != nullptr) // found
    return;

  std::string msg = pre + hostname + post + " Existing netpoints: \n";

  std::vector<simgrid::kernel::routing::NetPoint*> netpoints =
      simgrid::s4u::Engine::get_instance()->get_all_netpoints();
  std::sort(netpoints.begin(), netpoints.end(),
            [](simgrid::kernel::routing::NetPoint* a, simgrid::kernel::routing::NetPoint* b) {
              return a->get_name() < b->get_name();
            });
  bool first = true;
  for (auto const& np : netpoints) {
    if (np->is_netzone())
      continue;

    if (not first)
      msg += ",";
    first = false;
    msg += "'" + np->get_name() + "'";
    if (msg.length() > 4096) {
      msg.pop_back(); // remove trailing quote
      msg += "...(list truncated)......";
      break;
    }
  }
  surf_parse_error(msg);
}

double surf_parse_get_double(std::string s)
{
  try {
    return std::stod(s);
  } catch (std::invalid_argument& ia) {
    surf_parse_error(s + " is not a double");
    return -1;
  }
}

int surf_parse_get_int(std::string s)
{
  try {
    return std::stoi(s);
  } catch (std::invalid_argument& ia) {
    surf_parse_error(s + " is not a double");
    return -1;
  }
}

namespace {

/* Turn something like "1-4,6,9-11" into the vector {1,2,3,4,6,9,10,11} */
std::vector<int>* explodesRadical(std::string radicals)
{
  std::vector<int>* exploded = new std::vector<int>();

  // Make all hosts
  std::vector<std::string> radical_elements;
  boost::split(radical_elements, radicals, boost::is_any_of(","));
  for (auto const& group : radical_elements) {
    std::vector<std::string> radical_ends;
    boost::split(radical_ends, group, boost::is_any_of("-"));
    int start = surf_parse_get_int(radical_ends.front());
    int end   = 0;

    switch (radical_ends.size()) {
      case 1:
        end = start;
        break;
      case 2:
        end = surf_parse_get_int(radical_ends.back());
        break;
      default:
        surf_parse_error(std::string("Malformed radical: ") + group);
        break;
    }
    for (int i = start; i <= end; i++)
      exploded->push_back(i);
  }

  return exploded;
}

class unit_scale : public std::unordered_map<std::string, double> {
public:
  using std::unordered_map<std::string, double>::unordered_map;
  // tuples are : <unit, value for unit, base (2 or 10), true if abbreviated>
  explicit unit_scale(std::initializer_list<std::tuple<const std::string, double, int, bool>> generators);
};

unit_scale::unit_scale(std::initializer_list<std::tuple<const std::string, double, int, bool>> generators)
{
  for (const auto& gen : generators) {
    const std::string& unit = std::get<0>(gen);
    double value            = std::get<1>(gen);
    const int base          = std::get<2>(gen);
    const bool abbrev       = std::get<3>(gen);
    double mult;
    std::vector<std::string> prefixes;
    switch (base) {
      case 2:
        mult     = 1024.0;
        prefixes = abbrev ? std::vector<std::string>{"Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi", "Yi"}
                          : std::vector<std::string>{"kibi", "mebi", "gibi", "tebi", "pebi", "exbi", "zebi", "yobi"};
        break;
      case 10:
        mult     = 1000.0;
        prefixes = abbrev ? std::vector<std::string>{"k", "M", "G", "T", "P", "E", "Z", "Y"}
                          : std::vector<std::string>{"kilo", "mega", "giga", "tera", "peta", "exa", "zeta", "yotta"};
        break;
      default:
        THROW_IMPOSSIBLE;
    }
    emplace(unit, value);
    for (const auto& prefix : prefixes) {
      value *= mult;
      emplace(prefix + unit, value);
    }
  }
}

/* Note: field `unit' for the last element of parameter `units' should be nullptr. */
double surf_parse_get_value_with_unit(const char* string, const unit_scale& units, const char* entity_kind,
                                      std::string name, const char* error_msg, const char* default_unit)
{
  char* ptr;
  errno = 0;
  double res   = strtod(string, &ptr);
  if (errno == ERANGE)
    surf_parse_error(std::string("value out of range: ") + string);
  if (ptr == string)
    surf_parse_error(std::string("cannot parse number:") + string);
  if (ptr[0] == '\0') {
    if (res == 0)
      return res; // Ok, 0 can be unit-less

    XBT_WARN("Deprecated unit-less value '%s' for %s %s. %s", string, entity_kind, name.c_str(), error_msg);
    ptr = (char*)default_unit;
  }
  auto u = units.find(ptr);
  if (u == units.end())
    surf_parse_error(std::string("unknown unit: ") + ptr);
  return res * u->second;
}
}

double surf_parse_get_time(const char* string, const char* entity_kind, std::string name)
{
  static const unit_scale units{std::make_pair("w", 7 * 24 * 60 * 60),
                                std::make_pair("d", 24 * 60 * 60),
                                std::make_pair("h", 60 * 60),
                                std::make_pair("m", 60),
                                std::make_pair("s", 1.0),
                                std::make_pair("ms", 1e-3),
                                std::make_pair("us", 1e-6),
                                std::make_pair("ns", 1e-9),
                                std::make_pair("ps", 1e-12)};
  return surf_parse_get_value_with_unit(string, units, entity_kind, name,
      "Append 's' to your time to get seconds", "s");
}

double surf_parse_get_size(const char* string, const char* entity_kind, std::string name)
{
  static const unit_scale units{std::make_tuple("b", 0.125, 2, true), std::make_tuple("b", 0.125, 10, true),
                                std::make_tuple("B", 1.0, 2, true), std::make_tuple("B", 1.0, 10, true)};
  return surf_parse_get_value_with_unit(string, units, entity_kind, name,
      "Append 'B' to get bytes (or 'b' for bits but 1B = 8b).", "B");
}

double surf_parse_get_bandwidth(const char* string, const char* entity_kind, std::string name)
{
  static const unit_scale units{std::make_tuple("bps", 0.125, 2, true), std::make_tuple("bps", 0.125, 10, true),
                                std::make_tuple("Bps", 1.0, 2, true), std::make_tuple("Bps", 1.0, 10, true)};
  return surf_parse_get_value_with_unit(string, units, entity_kind, name,
      "Append 'Bps' to get bytes per second (or 'bps' for bits but 1Bps = 8bps)", "Bps");
}

double surf_parse_get_speed(const char* string, const char* entity_kind, std::string name)
{
  static const unit_scale units{std::make_tuple("f", 1.0, 10, true), std::make_tuple("flops", 1.0, 10, false)};
  return surf_parse_get_value_with_unit(string, units, entity_kind, name,
      "Append 'f' or 'flops' to your speed to get flop per second", "f");
}

static std::vector<double> surf_parse_get_all_speeds(char* speeds, const char* entity_kind, std::string id)
{

  std::vector<double> speed_per_pstate;

  if (strchr(speeds, ',') == nullptr){
    double speed = surf_parse_get_speed(speeds, entity_kind, id);
    speed_per_pstate.push_back(speed);
  } else {
    std::vector<std::string> pstate_list;
    boost::split(pstate_list, speeds, boost::is_any_of(","));
    for (auto speed_str : pstate_list) {
      boost::trim(speed_str);
      double speed = surf_parse_get_speed(speed_str.c_str(), entity_kind, id);
      speed_per_pstate.push_back(speed);
      XBT_DEBUG("Speed value: %f", speed);
    }
  }
  return speed_per_pstate;
}

/*
 * All the callback lists that can be overridden anywhere.
 * (this list should probably be reduced to the bare minimum to allow the models to work)
 */

/* make sure these symbols are defined as strong ones in this file so that the linker can resolve them */

/* The default current property receiver. Setup in the corresponding opening callbacks. */
std::unordered_map<std::string, std::string>* current_property_set       = nullptr;
std::unordered_map<std::string, std::string>* current_model_property_set = nullptr;
int ZONE_TAG                            = 0; // Whether we just opened a zone tag (to see what to do with the properties)

FILE *surf_file_to_parse = nullptr;

/* Stuff relative to storage */
void STag_surfxml_storage()
{
  ZONE_TAG = 0;
  XBT_DEBUG("STag_surfxml_storage");
  xbt_assert(current_property_set == nullptr, "Someone forgot to reset the property set to nullptr in its closing tag (or XML malformed)");
}

void ETag_surfxml_storage()
{
  simgrid::kernel::routing::StorageCreationArgs storage;

  storage.properties   = current_property_set;
  current_property_set = nullptr;

  storage.id           = A_surfxml_storage_id;
  storage.type_id      = A_surfxml_storage_typeId;
  storage.content      = A_surfxml_storage_content;
  storage.attach       = A_surfxml_storage_attach;

  sg_platf_new_storage(&storage);
}
void STag_surfxml_storage___type()
{
  ZONE_TAG = 0;
  XBT_DEBUG("STag_surfxml_storage___type");
  xbt_assert(current_property_set == nullptr, "Someone forgot to reset the property set to nullptr in its closing tag (or XML malformed)");
  xbt_assert(current_model_property_set == nullptr, "Someone forgot to reset the model property set to nullptr in its closing tag (or XML malformed)");
}
void ETag_surfxml_storage___type()
{
  simgrid::kernel::routing::StorageTypeCreationArgs storage_type;

  storage_type.properties = current_property_set;
  current_property_set    = nullptr;

  storage_type.model_properties = current_model_property_set;
  current_model_property_set    = nullptr;

  storage_type.content = A_surfxml_storage___type_content;
  storage_type.id      = A_surfxml_storage___type_id;
  storage_type.model   = A_surfxml_storage___type_model;
  storage_type.size =
      surf_parse_get_size(A_surfxml_storage___type_size, "size of storage type", storage_type.id.c_str());
  sg_platf_new_storage_type(&storage_type);
}

void STag_surfxml_mount()
{
  XBT_DEBUG("STag_surfxml_mount");
}

void ETag_surfxml_mount()
{
  simgrid::kernel::routing::MountCreationArgs mount;

  mount.name      = A_surfxml_mount_name;
  mount.storageId = A_surfxml_mount_storageId;
  sg_platf_new_mount(&mount);
}

void STag_surfxml_include()
{
  xbt_die("<include> tag was removed in SimGrid v3.18. Please stop using it now.");
}

void ETag_surfxml_include()
{
  /* Won't happen since <include> is now removed since v3.18. */
}

/* Stag and Etag parse functions */
void STag_surfxml_platform() {
  XBT_ATTRIB_UNUSED double version = surf_parse_get_double(A_surfxml_platform_version);

  xbt_assert((version >= 1.0), "******* BIG FAT WARNING *********\n "
      "You're using an ancient XML file.\n"
      "Since SimGrid 3.1, units are Bytes, Flops, and seconds "
      "instead of MBytes, MFlops and seconds.\n"

      "Use simgrid_update_xml to update your file automatically. "
      "This program is installed automatically with SimGrid, or "
      "available in the tools/ directory of the source archive.\n"

      "Please check also out the SURF section of the ChangeLog for "
      "the 3.1 version for more information. \n"

      "Last, do not forget to also update your values for "
      "the calls to MSG_task_create (if any).");
  xbt_assert((version >= 3.0), "******* BIG FAT WARNING *********\n "
      "You're using an old XML file.\n"
      "Use simgrid_update_xml to update your file automatically. "
      "This program is installed automatically with SimGrid, or "
      "available in the tools/ directory of the source archive.");
  xbt_assert((version >= 4.0),
             "******* FILE %s IS TOO OLD (v:%.1f) *********\n "
             "Changes introduced in SimGrid 3.13:\n"
             "  - 'power' attribute of hosts (and others) got renamed to 'speed'.\n"
             "  - In <trace_connect>, attribute kind=\"POWER\" is now kind=\"SPEED\".\n"
             "  - DOCTYPE now point to the rignt URL: http://simgrid.gforge.inria.fr/simgrid/simgrid.dtd\n"
             "  - speed, bandwidth and latency attributes now MUST have an explicit unit (f, Bps, s by default)"
             "\n\n"
             "Use simgrid_update_xml to update your file automatically. "
             "This program is installed automatically with SimGrid, or "
             "available in the tools/ directory of the source archive.",
             surf_parsed_filename.c_str(), version);
  if (version < 4.1) {
    XBT_INFO("You're using a v%.1f XML file (%s) while the current standard is v4.1 "
             "That's fine, the new version is backward compatible. \n\n"
             "Use simgrid_update_xml to update your file automatically to get rid of this warning. "
             "This program is installed automatically with SimGrid, or "
             "available in the tools/ directory of the source archive.",
             version, surf_parsed_filename.c_str());
  }
  xbt_assert(version <= 4.1,
             "******* FILE %s COMES FROM THE FUTURE (v:%.1f) *********\n "
             "The most recent formalism that this version of SimGrid understands is v4.1.\n"
             "Please update your code, or use another, more adapted, file.",
             surf_parsed_filename.c_str(), version);
}
void ETag_surfxml_platform(){
  simgrid::s4u::on_platform_created();
}

void STag_surfxml_host(){
  ZONE_TAG = 0;
  xbt_assert(current_property_set == nullptr, "Someone forgot to reset the property set to nullptr in its closing tag (or XML malformed)");
}

void STag_surfxml_prop()
{
  if (ZONE_TAG) { // We need to retrieve the most recently opened zone
    XBT_DEBUG("Set zone property %s -> %s", A_surfxml_prop_id, A_surfxml_prop_value);
    simgrid::s4u::NetZone* netzone = simgrid::s4u::Engine::get_instance()->netzone_by_name_or_null(A_surfxml_zone_id);

    netzone->set_property(std::string(A_surfxml_prop_id), A_surfxml_prop_value);
  } else {
    if (not current_property_set)
      current_property_set = new std::unordered_map<std::string, std::string>; // Maybe, it should raise an error
    current_property_set->insert({A_surfxml_prop_id, A_surfxml_prop_value});
    XBT_DEBUG("add prop %s=%s into current property set %p", A_surfxml_prop_id, A_surfxml_prop_value,
              current_property_set);
  }
}

void ETag_surfxml_host()    {
  simgrid::kernel::routing::HostCreationArgs host;

  host.properties = current_property_set;
  current_property_set = nullptr;

  host.id = A_surfxml_host_id;

  host.speed_per_pstate = surf_parse_get_all_speeds(A_surfxml_host_speed, "speed of host", host.id);

  XBT_DEBUG("pstate: %s", A_surfxml_host_pstate);
  host.core_amount = surf_parse_get_int(A_surfxml_host_core);
  host.speed_trace = A_surfxml_host_availability___file[0] ? tmgr_trace_new_from_file(A_surfxml_host_availability___file) : nullptr;
  host.state_trace = A_surfxml_host_state___file[0] ? tmgr_trace_new_from_file(A_surfxml_host_state___file) : nullptr;
  host.pstate      = surf_parse_get_int(A_surfxml_host_pstate);
  host.coord       = A_surfxml_host_coordinates;

  sg_platf_new_host(&host);
}

void STag_surfxml_host___link(){
  XBT_DEBUG("Create a Host_link for %s",A_surfxml_host___link_id);
  simgrid::kernel::routing::HostLinkCreationArgs host_link;

  host_link.id        = A_surfxml_host___link_id;
  host_link.link_up   = A_surfxml_host___link_up;
  host_link.link_down = A_surfxml_host___link_down;
  sg_platf_new_hostlink(&host_link);
}

void STag_surfxml_router(){
  sg_platf_new_router(A_surfxml_router_id, A_surfxml_router_coordinates);
}

void ETag_surfxml_cluster(){
  simgrid::kernel::routing::ClusterCreationArgs cluster;
  cluster.properties   = current_property_set;
  current_property_set = nullptr;

  cluster.id          = A_surfxml_cluster_id;
  cluster.prefix      = A_surfxml_cluster_prefix;
  cluster.suffix      = A_surfxml_cluster_suffix;
  cluster.radicals    = explodesRadical(A_surfxml_cluster_radical);
  cluster.speeds      = surf_parse_get_all_speeds(A_surfxml_cluster_speed, "speed of cluster", cluster.id);
  cluster.core_amount = surf_parse_get_int(A_surfxml_cluster_core);
  cluster.bw          = surf_parse_get_bandwidth(A_surfxml_cluster_bw, "bw of cluster", cluster.id);
  cluster.lat         = surf_parse_get_time(A_surfxml_cluster_lat, "lat of cluster", cluster.id);
  if(strcmp(A_surfxml_cluster_bb___bw,""))
    cluster.bb_bw = surf_parse_get_bandwidth(A_surfxml_cluster_bb___bw, "bb_bw of cluster", cluster.id);
  if(strcmp(A_surfxml_cluster_bb___lat,""))
    cluster.bb_lat = surf_parse_get_time(A_surfxml_cluster_bb___lat, "bb_lat of cluster", cluster.id);
  if(strcmp(A_surfxml_cluster_limiter___link,""))
    cluster.limiter_link = surf_parse_get_bandwidth(A_surfxml_cluster_limiter___link, "limiter_link of cluster", cluster.id);
  if(strcmp(A_surfxml_cluster_loopback___bw,""))
    cluster.loopback_bw = surf_parse_get_bandwidth(A_surfxml_cluster_loopback___bw, "loopback_bw of cluster", cluster.id);
  if(strcmp(A_surfxml_cluster_loopback___lat,""))
    cluster.loopback_lat = surf_parse_get_time(A_surfxml_cluster_loopback___lat, "loopback_lat of cluster", cluster.id);

  switch(AX_surfxml_cluster_topology){
  case A_surfxml_cluster_topology_FLAT:
    cluster.topology = simgrid::kernel::routing::ClusterTopology::FLAT;
    break;
  case A_surfxml_cluster_topology_TORUS:
    cluster.topology = simgrid::kernel::routing::ClusterTopology::TORUS;
    break;
  case A_surfxml_cluster_topology_FAT___TREE:
    cluster.topology = simgrid::kernel::routing::ClusterTopology::FAT_TREE;
    break;
  case A_surfxml_cluster_topology_DRAGONFLY:
    cluster.topology = simgrid::kernel::routing::ClusterTopology::DRAGONFLY;
    break;
  default:
    surf_parse_error(std::string("Invalid cluster topology for cluster ") + cluster.id);
    break;
  }
  cluster.topo_parameters = A_surfxml_cluster_topo___parameters;
  cluster.router_id = A_surfxml_cluster_router___id;

  switch (AX_surfxml_cluster_sharing___policy) {
  case A_surfxml_cluster_sharing___policy_SHARED:
    cluster.sharing_policy = simgrid::s4u::Link::SharingPolicy::SHARED;
    break;
  case A_surfxml_cluster_sharing___policy_FULLDUPLEX:
    XBT_WARN("FULLDUPLEX is now deprecated. Please update your platform file to use SPLITDUPLEX instead.");
    cluster.sharing_policy = simgrid::s4u::Link::SharingPolicy::SPLITDUPLEX;
    break;
  case A_surfxml_cluster_sharing___policy_SPLITDUPLEX:
    cluster.sharing_policy = simgrid::s4u::Link::SharingPolicy::SPLITDUPLEX;
    break;
  case A_surfxml_cluster_sharing___policy_FATPIPE:
    cluster.sharing_policy = simgrid::s4u::Link::SharingPolicy::FATPIPE;
    break;
  default:
    surf_parse_error(std::string("Invalid cluster sharing policy for cluster ") + cluster.id);
    break;
  }
  switch (AX_surfxml_cluster_bb___sharing___policy) {
  case A_surfxml_cluster_bb___sharing___policy_FATPIPE:
    cluster.bb_sharing_policy = simgrid::s4u::Link::SharingPolicy::FATPIPE;
    break;
  case A_surfxml_cluster_bb___sharing___policy_SHARED:
    cluster.bb_sharing_policy = simgrid::s4u::Link::SharingPolicy::SHARED;
    break;
  default:
    surf_parse_error(std::string("Invalid bb sharing policy in cluster ") + cluster.id);
    break;
  }

  sg_platf_new_cluster(&cluster);
}

void STag_surfxml_cluster(){
  ZONE_TAG = 0;
  xbt_assert(current_property_set == nullptr, "Someone forgot to reset the property set to nullptr in its closing tag (or XML malformed)");
}

void STag_surfxml_cabinet(){
  simgrid::kernel::routing::CabinetCreationArgs cabinet;
  cabinet.id      = A_surfxml_cabinet_id;
  cabinet.prefix  = A_surfxml_cabinet_prefix;
  cabinet.suffix  = A_surfxml_cabinet_suffix;
  cabinet.speed    = surf_parse_get_speed(A_surfxml_cabinet_speed, "speed of cabinet", cabinet.id.c_str());
  cabinet.bw       = surf_parse_get_bandwidth(A_surfxml_cabinet_bw, "bw of cabinet", cabinet.id.c_str());
  cabinet.lat      = surf_parse_get_time(A_surfxml_cabinet_lat, "lat of cabinet", cabinet.id.c_str());
  cabinet.radicals = explodesRadical(A_surfxml_cabinet_radical);

  sg_platf_new_cabinet(&cabinet);
}

void STag_surfxml_peer(){
  simgrid::kernel::routing::PeerCreationArgs peer;

  peer.id          = std::string(A_surfxml_peer_id);
  peer.speed       = surf_parse_get_speed(A_surfxml_peer_speed, "speed of peer", peer.id.c_str());
  peer.bw_in       = surf_parse_get_bandwidth(A_surfxml_peer_bw___in, "bw_in of peer", peer.id.c_str());
  peer.bw_out      = surf_parse_get_bandwidth(A_surfxml_peer_bw___out, "bw_out of peer", peer.id.c_str());
  peer.coord       = A_surfxml_peer_coordinates;
  peer.speed_trace = A_surfxml_peer_availability___file[0] ? tmgr_trace_new_from_file(A_surfxml_peer_availability___file) : nullptr;
  peer.state_trace = A_surfxml_peer_state___file[0] ? tmgr_trace_new_from_file(A_surfxml_peer_state___file) : nullptr;

  if (A_surfxml_peer_lat[0] != '\0')
    XBT_WARN("The latency parameter in <peer> is now deprecated. Use the z coordinate instead of '%s'.",
             A_surfxml_peer_lat);

  sg_platf_new_peer(&peer);
}

void STag_surfxml_link(){
  ZONE_TAG = 0;
  xbt_assert(current_property_set == nullptr, "Someone forgot to reset the property set to nullptr in its closing tag (or XML malformed)");
}

void ETag_surfxml_link(){
  simgrid::kernel::routing::LinkCreationArgs link;

  link.properties          = current_property_set;
  current_property_set     = nullptr;

  link.id                  = std::string(A_surfxml_link_id);
  link.bandwidth           = surf_parse_get_bandwidth(A_surfxml_link_bandwidth, "bandwidth of link", link.id.c_str());
  link.bandwidth_trace     = A_surfxml_link_bandwidth___file[0] ? tmgr_trace_new_from_file(A_surfxml_link_bandwidth___file) : nullptr;
  link.latency             = surf_parse_get_time(A_surfxml_link_latency, "latency of link", link.id.c_str());
  link.latency_trace       = A_surfxml_link_latency___file[0] ? tmgr_trace_new_from_file(A_surfxml_link_latency___file) : nullptr;
  link.state_trace         = A_surfxml_link_state___file[0] ? tmgr_trace_new_from_file(A_surfxml_link_state___file):nullptr;

  switch (A_surfxml_link_sharing___policy) {
  case A_surfxml_link_sharing___policy_SHARED:
    link.policy = simgrid::s4u::Link::SharingPolicy::SHARED;
    break;
  case A_surfxml_link_sharing___policy_FATPIPE:
    link.policy = simgrid::s4u::Link::SharingPolicy::FATPIPE;
    break;
  case A_surfxml_link_sharing___policy_FULLDUPLEX:
    XBT_WARN("FULLDUPLEX is now deprecated. Please update your platform file to use SPLITDUPLEX instead.");
    link.policy = simgrid::s4u::Link::SharingPolicy::SPLITDUPLEX;
    break;
  case A_surfxml_link_sharing___policy_SPLITDUPLEX:
    link.policy = simgrid::s4u::Link::SharingPolicy::SPLITDUPLEX;
    break;
  default:
    surf_parse_error(std::string("Invalid sharing policy in link ") + link.id);
    break;
  }

  sg_platf_new_link(&link);
}

void STag_surfxml_link___ctn()
{
  simgrid::kernel::resource::LinkImpl* link = nullptr;
  switch (A_surfxml_link___ctn_direction) {
  case AU_surfxml_link___ctn_direction:
  case A_surfxml_link___ctn_direction_NONE:
    link = simgrid::s4u::Link::by_name(std::string(A_surfxml_link___ctn_id))->get_impl();
    break;
  case A_surfxml_link___ctn_direction_UP:
    link = simgrid::s4u::Link::by_name(std::string(A_surfxml_link___ctn_id) + "_UP")->get_impl();
    break;
  case A_surfxml_link___ctn_direction_DOWN:
    link = simgrid::s4u::Link::by_name(std::string(A_surfxml_link___ctn_id) + "_DOWN")->get_impl();
    break;
  default:
    surf_parse_error(std::string("Invalid direction for link ") + A_surfxml_link___ctn_id);
    break;
  }

  const char* dirname = "";
  switch (A_surfxml_link___ctn_direction) {
    case A_surfxml_link___ctn_direction_UP:
      dirname = " (upward)";
      break;
    case A_surfxml_link___ctn_direction_DOWN:
      dirname = " (downward)";
      break;
    default:
      dirname = "";
  }
  surf_parse_assert(link != nullptr, std::string("No such link: '") + A_surfxml_link___ctn_id + "'" + dirname);
  parsed_link_list.push_back(link);
}

void ETag_surfxml_backbone(){
  simgrid::kernel::routing::LinkCreationArgs link;

  link.properties = nullptr;
  link.id = std::string(A_surfxml_backbone_id);
  link.bandwidth = surf_parse_get_bandwidth(A_surfxml_backbone_bandwidth, "bandwidth of backbone", link.id.c_str());
  link.latency = surf_parse_get_time(A_surfxml_backbone_latency, "latency of backbone", link.id.c_str());
  link.policy     = simgrid::s4u::Link::SharingPolicy::SHARED;

  sg_platf_new_link(&link);
  routing_cluster_add_backbone(simgrid::s4u::Link::by_name(std::string(A_surfxml_backbone_id))->get_impl());
}

void STag_surfxml_route(){
  surf_parse_assert_netpoint(A_surfxml_route_src, "Route src='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_route_dst, "Route dst='", "' does name a node.");
}

void STag_surfxml_ASroute(){
  surf_parse_assert_netpoint(A_surfxml_ASroute_src, "ASroute src='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_ASroute_dst, "ASroute dst='", "' does name a node.");

  surf_parse_assert_netpoint(A_surfxml_ASroute_gw___src, "ASroute gw_src='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_ASroute_gw___dst, "ASroute gw_dst='", "' does name a node.");
}
void STag_surfxml_zoneRoute(){
  surf_parse_assert_netpoint(A_surfxml_zoneRoute_src, "zoneRoute src='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_zoneRoute_dst, "zoneRoute dst='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_zoneRoute_gw___src, "zoneRoute gw_src='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_zoneRoute_gw___dst, "zoneRoute gw_dst='", "' does name a node.");
}

void STag_surfxml_bypassRoute(){
  surf_parse_assert_netpoint(A_surfxml_bypassRoute_src, "bypassRoute src='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_bypassRoute_dst, "bypassRoute dst='", "' does name a node.");
}

void STag_surfxml_bypassASroute(){
  surf_parse_assert_netpoint(A_surfxml_bypassASroute_src, "bypassASroute src='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_bypassASroute_dst, "bypassASroute dst='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_bypassASroute_gw___src, "bypassASroute gw_src='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_bypassASroute_gw___dst, "bypassASroute gw_dst='", "' does name a node.");
}
void STag_surfxml_bypassZoneRoute(){
  surf_parse_assert_netpoint(A_surfxml_bypassZoneRoute_src, "bypassZoneRoute src='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_bypassZoneRoute_dst, "bypassZoneRoute dst='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_bypassZoneRoute_gw___src, "bypassZoneRoute gw_src='", "' does name a node.");
  surf_parse_assert_netpoint(A_surfxml_bypassZoneRoute_gw___dst, "bypassZoneRoute gw_dst='", "' does name a node.");
}

void ETag_surfxml_route(){
  simgrid::kernel::routing::RouteCreationArgs route;

  route.src         = sg_netpoint_by_name_or_null(A_surfxml_route_src); // tested to not be nullptr in start tag
  route.dst         = sg_netpoint_by_name_or_null(A_surfxml_route_dst); // tested to not be nullptr in start tag
  route.gw_src    = nullptr;
  route.gw_dst    = nullptr;
  route.symmetrical = (A_surfxml_route_symmetrical == A_surfxml_route_symmetrical_YES);

  route.link_list.swap(parsed_link_list);

  sg_platf_new_route(&route);
}

void ETag_surfxml_ASroute()
{
  AX_surfxml_zoneRoute_src = AX_surfxml_ASroute_src;
  AX_surfxml_zoneRoute_dst = AX_surfxml_ASroute_dst;
  AX_surfxml_zoneRoute_gw___src = AX_surfxml_ASroute_gw___src;
  AX_surfxml_zoneRoute_gw___dst = AX_surfxml_ASroute_gw___dst;
  AX_surfxml_zoneRoute_symmetrical = (AT_surfxml_zoneRoute_symmetrical)AX_surfxml_ASroute_symmetrical;
  ETag_surfxml_zoneRoute();
}
void ETag_surfxml_zoneRoute()
{
  simgrid::kernel::routing::RouteCreationArgs ASroute;

  ASroute.src = sg_netpoint_by_name_or_null(A_surfxml_zoneRoute_src); // tested to not be nullptr in start tag
  ASroute.dst = sg_netpoint_by_name_or_null(A_surfxml_zoneRoute_dst); // tested to not be nullptr in start tag

  ASroute.gw_src = sg_netpoint_by_name_or_null(A_surfxml_zoneRoute_gw___src); // tested to not be nullptr in start tag
  ASroute.gw_dst = sg_netpoint_by_name_or_null(A_surfxml_zoneRoute_gw___dst); // tested to not be nullptr in start tag

  ASroute.link_list.swap(parsed_link_list);

  switch (A_surfxml_zoneRoute_symmetrical) {
  case AU_surfxml_zoneRoute_symmetrical:
  case A_surfxml_zoneRoute_symmetrical_YES:
    ASroute.symmetrical = true;
    break;
  case A_surfxml_zoneRoute_symmetrical_NO:
    ASroute.symmetrical = false;
    break;
  default:
    THROW_IMPOSSIBLE;
  }

  sg_platf_new_route(&ASroute);
}

void ETag_surfxml_bypassRoute(){
  simgrid::kernel::routing::RouteCreationArgs route;

  route.src         = sg_netpoint_by_name_or_null(A_surfxml_bypassRoute_src); // tested to not be nullptr in start tag
  route.dst         = sg_netpoint_by_name_or_null(A_surfxml_bypassRoute_dst); // tested to not be nullptr in start tag
  route.gw_src = nullptr;
  route.gw_dst = nullptr;
  route.symmetrical = false;

  route.link_list.swap(parsed_link_list);

  sg_platf_new_bypassRoute(&route);
}

void ETag_surfxml_bypassASroute()
{
  AX_surfxml_bypassZoneRoute_src = AX_surfxml_bypassASroute_src;
  AX_surfxml_bypassZoneRoute_dst = AX_surfxml_bypassASroute_dst;
  AX_surfxml_bypassZoneRoute_gw___src = AX_surfxml_bypassASroute_gw___src;
  AX_surfxml_bypassZoneRoute_gw___dst = AX_surfxml_bypassASroute_gw___dst;
  ETag_surfxml_bypassZoneRoute();
}
void ETag_surfxml_bypassZoneRoute()
{
  simgrid::kernel::routing::RouteCreationArgs ASroute;

  ASroute.src         = sg_netpoint_by_name_or_null(A_surfxml_bypassZoneRoute_src);
  ASroute.dst         = sg_netpoint_by_name_or_null(A_surfxml_bypassZoneRoute_dst);
  ASroute.link_list.swap(parsed_link_list);

  ASroute.symmetrical = false;

  ASroute.gw_src = sg_netpoint_by_name_or_null(A_surfxml_bypassZoneRoute_gw___src);
  ASroute.gw_dst = sg_netpoint_by_name_or_null(A_surfxml_bypassZoneRoute_gw___dst);

  sg_platf_new_bypassRoute(&ASroute);
}

void ETag_surfxml_trace(){
  simgrid::kernel::routing::TraceCreationArgs trace;

  trace.id = A_surfxml_trace_id;
  trace.file = A_surfxml_trace_file;
  trace.periodicity = surf_parse_get_double(A_surfxml_trace_periodicity);
  trace.pc_data = surfxml_pcdata;

  sg_platf_new_trace(&trace);
}

void STag_surfxml_trace___connect()
{
  simgrid::kernel::routing::TraceConnectCreationArgs trace_connect;

  trace_connect.element = A_surfxml_trace___connect_element;
  trace_connect.trace = A_surfxml_trace___connect_trace;

  switch (A_surfxml_trace___connect_kind) {
  case AU_surfxml_trace___connect_kind:
  case A_surfxml_trace___connect_kind_SPEED:
    trace_connect.kind = simgrid::kernel::routing::TraceConnectKind::SPEED;
    break;
  case A_surfxml_trace___connect_kind_BANDWIDTH:
    trace_connect.kind = simgrid::kernel::routing::TraceConnectKind::BANDWIDTH;
    break;
  case A_surfxml_trace___connect_kind_HOST___AVAIL:
    trace_connect.kind = simgrid::kernel::routing::TraceConnectKind::HOST_AVAIL;
    break;
  case A_surfxml_trace___connect_kind_LATENCY:
    trace_connect.kind = simgrid::kernel::routing::TraceConnectKind::LATENCY;
    break;
  case A_surfxml_trace___connect_kind_LINK___AVAIL:
    trace_connect.kind = simgrid::kernel::routing::TraceConnectKind::LINK_AVAIL;
    break;
  default:
    surf_parse_error("Invalid trace kind");
    break;
  }
  sg_platf_trace_connect(&trace_connect);
}

void STag_surfxml_AS()
{
  AX_surfxml_zone_id = AX_surfxml_AS_id;
  AX_surfxml_zone_routing = (AT_surfxml_zone_routing)AX_surfxml_AS_routing;
  STag_surfxml_zone();
}

void ETag_surfxml_AS()
{
  ETag_surfxml_zone();
}

void STag_surfxml_zone()
{
  ZONE_TAG                 = 1;
  simgrid::kernel::routing::ZoneCreationArgs zone;
  zone.id      = A_surfxml_zone_id;
  zone.routing = static_cast<int>(A_surfxml_zone_routing);

  sg_platf_new_Zone_begin(&zone);
}

void ETag_surfxml_zone()
{
  sg_platf_new_Zone_seal();
}

void STag_surfxml_config()
{
  ZONE_TAG = 0;
  xbt_assert(current_property_set == nullptr,
             "Someone forgot to reset the property set to nullptr in its closing tag (or XML malformed)");
  XBT_DEBUG("START configuration name = %s",A_surfxml_config_id);
  if (_sg_cfg_init_status == 2) {
    surf_parse_error("All <config> tags must be given before any platform elements (such as <zone>, <host>, <cluster>, "
                     "<link>, etc).");
  }
}

void ETag_surfxml_config()
{
  // Sort config elements before applying.
  // That's a little waste of time, but not doing so would break the tests
  std::vector<std::string> keys;
  for (auto const& kv : *current_property_set) {
    keys.push_back(kv.first);
  }
  std::sort(keys.begin(), keys.end());
  for (std::string key : keys) {
    if (simgrid::config::is_default(key.c_str())) {
      std::string cfg = key + ":" + current_property_set->at(key);
      simgrid::config::set_parse(std::move(cfg));
    } else
      XBT_INFO("The custom configuration '%s' is already defined by user!", key.c_str());
  }
  XBT_DEBUG("End configuration name = %s",A_surfxml_config_id);

  delete current_property_set;
  current_property_set = nullptr;
}

static std::vector<std::string> arguments;

void STag_surfxml_process()
{
  AX_surfxml_actor_function = AX_surfxml_process_function;
  STag_surfxml_actor();
}

void STag_surfxml_actor()
{
  ZONE_TAG  = 0;
  arguments.assign(1, A_surfxml_actor_function);
  xbt_assert(current_property_set == nullptr, "Someone forgot to reset the property set to nullptr in its closing tag (or XML malformed)");
}

void ETag_surfxml_process()
{
  AX_surfxml_actor_host = AX_surfxml_process_host;
  AX_surfxml_actor_function = AX_surfxml_process_function;
  AX_surfxml_actor_start___time = AX_surfxml_process_start___time;
  AX_surfxml_actor_kill___time = AX_surfxml_process_kill___time;
  AX_surfxml_actor_on___failure = (AT_surfxml_actor_on___failure)AX_surfxml_process_on___failure;
  ETag_surfxml_actor();
}

void ETag_surfxml_actor()
{
  simgrid::kernel::routing::ActorCreationArgs actor;

  actor.properties     = current_property_set;
  current_property_set = nullptr;

  actor.args.swap(arguments);
  actor.host       = A_surfxml_actor_host;
  actor.function   = A_surfxml_actor_function;
  actor.start_time = surf_parse_get_double(A_surfxml_actor_start___time);
  actor.kill_time  = surf_parse_get_double(A_surfxml_actor_kill___time);

  switch (A_surfxml_actor_on___failure) {
  case AU_surfxml_actor_on___failure:
  case A_surfxml_actor_on___failure_DIE:
    actor.on_failure = simgrid::kernel::routing::ActorOnFailure::DIE;
    break;
  case A_surfxml_actor_on___failure_RESTART:
    actor.on_failure = simgrid::kernel::routing::ActorOnFailure::RESTART;
    break;
  default:
    surf_parse_error("Invalid on failure behavior");
    break;
  }

  sg_platf_new_actor(&actor);
}

void STag_surfxml_argument(){
  arguments.push_back(A_surfxml_argument_value);
}

void STag_surfxml_model___prop(){
  if (not current_model_property_set)
    current_model_property_set = new std::unordered_map<std::string, std::string>();

  current_model_property_set->insert({A_surfxml_model___prop_id, A_surfxml_model___prop_value});
}

void ETag_surfxml_prop(){/* Nothing to do */}
void STag_surfxml_random(){/* Nothing to do */}
void ETag_surfxml_random(){/* Nothing to do */}
void ETag_surfxml_trace___connect(){/* Nothing to do */}
void STag_surfxml_trace()
{ /* Nothing to do */
}
void ETag_surfxml_router(){/*Nothing to do*/}
void ETag_surfxml_host___link(){/* Nothing to do */}
void ETag_surfxml_cabinet(){/* Nothing to do */}
void ETag_surfxml_peer(){/* Nothing to do */}
void STag_surfxml_backbone(){/* Nothing to do */}
void ETag_surfxml_link___ctn(){/* Nothing to do */}
void ETag_surfxml_argument(){/* Nothing to do */}
void ETag_surfxml_model___prop(){/* Nothing to do */}

/* Open and Close parse file */
YY_BUFFER_STATE surf_input_buffer;

void surf_parse_open(std::string file)
{
  surf_parsed_filename = file;
  std::string dir      = simgrid::xbt::Path(file).get_dir_name();
  surf_path.push_back(dir);

  surf_file_to_parse = surf_fopen(file, "r");
  if (surf_file_to_parse == nullptr)
    xbt_die("Unable to open '%s'\n", file.c_str());
  surf_input_buffer = surf_parse__create_buffer(surf_file_to_parse, YY_BUF_SIZE);
  surf_parse__switch_to_buffer(surf_input_buffer);
  surf_parse_lineno = 1;
}

void surf_parse_close()
{
  surf_path.pop_back(); // remove the dirname of the opened file, that was added in surf_parse_open()

  if (surf_file_to_parse) {
    surf_parse__delete_buffer(surf_input_buffer);
    fclose(surf_file_to_parse);
    surf_file_to_parse = nullptr; //Must be reset for Bypass
  }
}

/* Call the lexer to parse the currently opened file */
int surf_parse()
{
  return surf_parse_lex();
}
