/* Copyright (c) 2017-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/plugins/energy.h"
#include "simgrid/s4u/Engine.hpp"
#include "src/surf/network_interface.hpp"
#include "src/surf/surf_interface.hpp"
#include "surf/surf.hpp"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

SIMGRID_REGISTER_PLUGIN(link_energy, "Link energy consumption.", &sg_link_energy_plugin_init)

/** @addtogroup SURF_plugin_energy


 This is the link energy plugin, accounting for the dissipated energy in the simulated platform.

 The energy consumption of a link depends directly on its current traffic load. Specify that consumption in your
 platform file as follows:

 @verbatim
 <link id="SWITCH1" bandwidth="125Mbps" latency="5us" sharing_policy="SHARED" >
 <prop id="watt_range" value="100.0:200.0" />
 <prop id="watt_off" value="10" />
 </link>
 @endverbatim

 The first property means that when your link is switched on, but without anything to do, it will dissipate 100 Watts.
 If it's fully loaded, it will dissipate 200 Watts. If its load is at 50%, then it will dissipate 150 Watts.
 The second property means that when your host is turned off, it will dissipate only 10 Watts (please note that these
 values are arbitrary).

 To simulate the energy-related elements, first call the simgrid#energy#sg_link_energy_plugin_init() before your
 #MSG_init(),
 and then use the following function to retrieve the consumption of a given link: sg_link_get_consumed_energy().
 */

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(link_energy, surf, "Logging specific to the SURF LinkEnergy plugin");

namespace simgrid {
namespace plugin {

class LinkEnergy {
public:
  static simgrid::xbt::Extension<simgrid::s4u::Link, LinkEnergy> EXTENSION_ID;

  explicit LinkEnergy(simgrid::s4u::Link* ptr) : link_(ptr), last_updated_(surf_get_clock()) {}
  ~LinkEnergy() = default;

  void init_watts_range_list();
  double get_consumed_energy();
  void update();

private:
  double get_power();

  simgrid::s4u::Link* link_{};

  bool inited_{false};
  double idle_{0.0};
  double busy_{0.0};

  double total_energy_{0.0};
  double last_updated_{0.0}; /*< Timestamp of the last energy update event*/
};

simgrid::xbt::Extension<simgrid::s4u::Link, LinkEnergy> LinkEnergy::EXTENSION_ID;

void LinkEnergy::update()
{
  double power = get_power();
  double now   = surf_get_clock();
  total_energy_ += power * (now - last_updated_);
  last_updated_ = now;
}

void LinkEnergy::init_watts_range_list()
{

  if (inited_)
    return;
  inited_ = true;

  const char* all_power_values_str = this->link_->get_property("watt_range");

  if (all_power_values_str == nullptr)
    return;

  std::vector<std::string> all_power_values;
  boost::split(all_power_values, all_power_values_str, boost::is_any_of(","));

  for (auto current_power_values_str : all_power_values) {
    /* retrieve the power values associated */
    std::vector<std::string> current_power_values;
    boost::split(current_power_values, current_power_values_str, boost::is_any_of(":"));
    xbt_assert(current_power_values.size() == 2,
               "Power properties incorrectly defined - could not retrieve idle and busy power values for link %s",
               this->link_->get_cname());

    /* min_power corresponds to the idle power (link load = 0) */
    /* max_power is the power consumed at 100% link load       */
    try {
      idle_ = std::stod(current_power_values.front());
    } catch (std::invalid_argument& ia) {
      throw std::invalid_argument(std::string("Invalid idle power value for link ") + this->link_->get_cname());
    }

    try {
      busy_ = std::stod(current_power_values.back());
    } catch (std::invalid_argument& ia) {
      throw std::invalid_argument(std::string("Invalid busy power value for link ") + this->link_->get_cname());
    }

    update();
  }
}

double LinkEnergy::get_power()
{

  if (!inited_)
    return 0.0;

  double power_slope = busy_ - idle_;

  double normalized_link_usage = link_->get_usage() / link_->get_bandwidth();
  double dynamic_power         = power_slope * normalized_link_usage;

  return idle_ + dynamic_power;
}

double LinkEnergy::get_consumed_energy()
{
  if (last_updated_ < surf_get_clock()) // We need to simcall this as it modifies the environment
    simgrid::simix::simcall(std::bind(&LinkEnergy::update, this));
  return this->total_energy_;
}
} // namespace plugin
} // namespace simgrid

using simgrid::plugin::LinkEnergy;

/* **************************** events  callback *************************** */
static void on_communicate(simgrid::kernel::resource::NetworkAction* action, simgrid::s4u::Host*, simgrid::s4u::Host*)
{
  XBT_DEBUG("onCommunicate is called");
  for (simgrid::kernel::resource::LinkImpl* link : action->links()) {

    if (link == nullptr)
      continue;

    XBT_DEBUG("Update link %s", link->get_cname());
    LinkEnergy* link_energy = link->piface_.extension<LinkEnergy>();
    link_energy->init_watts_range_list();
    link_energy->update();
  }
}

static void on_simulation_end()
{
  std::vector<simgrid::s4u::Link*> links = simgrid::s4u::Engine::get_instance()->get_all_links();

  double total_energy = 0.0; // Total dissipated energy (whole platform)
  for (const auto link : links) {
    double link_energy = link->extension<LinkEnergy>()->get_consumed_energy();
    total_energy += link_energy;
  }

  XBT_INFO("Total energy over all links: %f", total_energy);
}
/* **************************** Public interface *************************** */

int sg_link_energy_is_inited()
{
  return LinkEnergy::EXTENSION_ID.valid();
}
/** @ingroup SURF_plugin_energy
 * @brief Enable energy plugin
 * @details Enable energy plugin to get joules consumption of each cpu. You should call this function before
 * #MSG_init().
 */
void sg_link_energy_plugin_init()
{

  if (LinkEnergy::EXTENSION_ID.valid())
    return;
  LinkEnergy::EXTENSION_ID = simgrid::s4u::Link::extension_create<LinkEnergy>();

  xbt_assert(sg_host_count() == 0, "Please call sg_link_energy_plugin_init() before initializing the platform.");

  simgrid::s4u::Link::on_creation.connect([](simgrid::s4u::Link& link) { link.extension_set(new LinkEnergy(&link)); });

  simgrid::s4u::Link::on_state_change.connect([](simgrid::s4u::Link& link) { link.extension<LinkEnergy>()->update(); });

  simgrid::s4u::Link::on_destruction.connect([](simgrid::s4u::Link& link) {
    if (strcmp(link.get_cname(), "__loopback__"))
      XBT_INFO("Energy consumption of link '%s': %f Joules", link.get_cname(),
               link.extension<LinkEnergy>()->get_consumed_energy());
  });

  simgrid::s4u::Link::on_communication_state_change.connect(
      [](simgrid::kernel::resource::NetworkAction* action, simgrid::kernel::resource::Action::State /* previous */) {
        for (simgrid::kernel::resource::LinkImpl* link : action->links()) {
          if (link != nullptr)
            link->piface_.extension<LinkEnergy>()->update();
        }
      });

  simgrid::s4u::Link::on_communicate.connect(&on_communicate);
  simgrid::s4u::on_simulation_end.connect(&on_simulation_end);
}

/** @ingroup plugin_energy
 *  @brief Returns the total energy consumed by the link so far (in Joules)
 *
 *  Please note that since the consumption is lazily updated, it may require a simcall to update it.
 *  The result is that the actor requesting this value will be interrupted,
 *  the value will be updated in kernel mode before returning the control to the requesting actor.
 */
double sg_link_get_consumed_energy(sg_link_t link)
{
  return link->extension<LinkEnergy>()->get_consumed_energy();
}
