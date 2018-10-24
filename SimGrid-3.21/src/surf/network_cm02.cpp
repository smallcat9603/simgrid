/* Copyright (c) 2013-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <algorithm>
#include <numeric>

#include "network_cm02.hpp"
#include "simgrid/s4u/Host.hpp"
#include "simgrid/sg_config.hpp"
#include "src/surf/surf_interface.hpp"
#include "surf/surf.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(surf_network);

double sg_latency_factor = 1.0; /* default value; can be set by model or from command line */
double sg_bandwidth_factor = 1.0;       /* default value; can be set by model or from command line */
double sg_weight_S_parameter = 0.0;     /* default value; can be set by model or from command line */

/************************************************************************/
/* New model based on optimizations discussed during Pedro Velho's thesis*/
/************************************************************************/
/* @techreport{VELHO:2011:HAL-00646896:1, */
/*      url = {http://hal.inria.fr/hal-00646896/en/}, */
/*      title = {{Flow-level network models: have we reached the limits?}}, */
/*      author = {Velho, Pedro and Schnorr, Lucas and Casanova, Henri and Legrand, Arnaud}, */
/*      type = {Rapport de recherche}, */
/*      institution = {INRIA}, */
/*      number = {RR-7821}, */
/*      year = {2011}, */
/*      month = Nov, */
/*      pdf = {http://hal.inria.fr/hal-00646896/PDF/rr-validity.pdf}, */
/*  } */
void surf_network_model_init_LegrandVelho()
{
  xbt_assert(surf_network_model == nullptr, "Cannot set the network model twice");

  surf_network_model = new simgrid::kernel::resource::NetworkCm02Model();

  simgrid::config::set_default<double>("network/latency-factor", 13.01);
  simgrid::config::set_default<double>("network/bandwidth-factor", 0.97);
  simgrid::config::set_default<double>("network/weight-S", 20537);
}

/***************************************************************************/
/* The nice TCP sharing model designed by Loris Marchal and Henri Casanova */
/***************************************************************************/
/* @TechReport{      rr-lip2002-40, */
/*   author        = {Henri Casanova and Loris Marchal}, */
/*   institution   = {LIP}, */
/*   title         = {A Network Model for Simulation of Grid Application}, */
/*   number        = {2002-40}, */
/*   month         = {oct}, */
/*   year          = {2002} */
/* } */
void surf_network_model_init_CM02()
{
  xbt_assert(surf_network_model == nullptr, "Cannot set the network model twice");

  simgrid::config::set_default<double>("network/latency-factor", 1.0);
  simgrid::config::set_default<double>("network/bandwidth-factor", 1.0);
  simgrid::config::set_default<double>("network/weight-S", 0.0);

  surf_network_model = new simgrid::kernel::resource::NetworkCm02Model();
}

/***************************************************************************/
/* The models from Steven H. Low                                           */
/***************************************************************************/
/* @article{Low03,                                                         */
/*   author={Steven H. Low},                                               */
/*   title={A Duality Model of {TCP} and Queue Management Algorithms},     */
/*   year={2003},                                                          */
/*   journal={{IEEE/ACM} Transactions on Networking},                      */
/*    volume={11}, number={4},                                             */
/*  }                                                                      */
void surf_network_model_init_Reno()
{
  xbt_assert(surf_network_model == nullptr, "Cannot set the network model twice");

  namespace lmm = simgrid::kernel::lmm;
  lmm::Lagrange::set_default_protocol_function(lmm::func_reno_f, lmm::func_reno_fp, lmm::func_reno_fpi);

  simgrid::config::set_default<double>("network/latency-factor", 13.01);
  simgrid::config::set_default<double>("network/bandwidth-factor", 0.97);
  simgrid::config::set_default<double>("network/weight-S", 20537);

  surf_network_model = new simgrid::kernel::resource::NetworkCm02Model(&simgrid::kernel::lmm::make_new_lagrange_system);
}


void surf_network_model_init_Reno2()
{
  xbt_assert(surf_network_model == nullptr, "Cannot set the network model twice");

  namespace lmm = simgrid::kernel::lmm;
  lmm::Lagrange::set_default_protocol_function(lmm::func_reno2_f, lmm::func_reno2_fp, lmm::func_reno2_fpi);

  simgrid::config::set_default<double>("network/latency-factor", 13.01);
  simgrid::config::set_default<double>("network/bandwidth-factor", 0.97);
  simgrid::config::set_default<double>("network/weight-S", 20537);

  surf_network_model = new simgrid::kernel::resource::NetworkCm02Model(&simgrid::kernel::lmm::make_new_lagrange_system);
}

void surf_network_model_init_Vegas()
{
  xbt_assert(surf_network_model == nullptr, "Cannot set the network model twice");

  namespace lmm = simgrid::kernel::lmm;
  lmm::Lagrange::set_default_protocol_function(lmm::func_vegas_f, lmm::func_vegas_fp, lmm::func_vegas_fpi);

  simgrid::config::set_default<double>("network/latency-factor", 13.01);
  simgrid::config::set_default<double>("network/bandwidth-factor", 0.97);
  simgrid::config::set_default<double>("network/weight-S", 20537);

  surf_network_model = new simgrid::kernel::resource::NetworkCm02Model(&simgrid::kernel::lmm::make_new_lagrange_system);
}

namespace simgrid {
namespace kernel {
namespace resource {

NetworkCm02Model::NetworkCm02Model(kernel::lmm::System* (*make_new_lmm_system)(bool))
    : NetworkModel(simgrid::config::get_value<std::string>("network/optim") == "Full" ? Model::UpdateAlgo::FULL
                                                                                      : Model::UpdateAlgo::LAZY)
{
  all_existing_models.push_back(this);

  std::string optim = simgrid::config::get_value<std::string>("network/optim");
  bool select       = simgrid::config::get_value<bool>("network/maxmin-selective-update");

  if (optim == "Lazy") {
    xbt_assert(select || simgrid::config::is_default("network/maxmin-selective-update"),
               "You cannot disable network selective update when using the lazy update mechanism");
    select = true;
  }

  set_maxmin_system(make_new_lmm_system(select));
  loopback_ = NetworkCm02Model::create_link("__loopback__", 498000000, 0.000015, s4u::Link::SharingPolicy::FATPIPE);
}

LinkImpl* NetworkCm02Model::create_link(const std::string& name, double bandwidth, double latency,
                                        s4u::Link::SharingPolicy policy)
{
  return new NetworkCm02Link(this, name, bandwidth, latency, policy, get_maxmin_system());
}

void NetworkCm02Model::update_actions_state_lazy(double now, double /*delta*/)
{
  while (not get_action_heap().empty() && double_equals(get_action_heap().top_date(), now, sg_surf_precision)) {

    NetworkCm02Action* action = static_cast<NetworkCm02Action*>(get_action_heap().pop());
    XBT_DEBUG("Something happened to action %p", action);

    // if I am wearing a latency hat
    if (action->get_type() == ActionHeap::Type::latency) {
      XBT_DEBUG("Latency paid for action %p. Activating", action);
      get_maxmin_system()->update_variable_weight(action->get_variable(), action->weight_);
      get_action_heap().remove(action);
      action->set_last_update();

      // if I am wearing a max_duration or normal hat
    } else if (action->get_type() == ActionHeap::Type::max_duration || action->get_type() == ActionHeap::Type::normal) {
      // no need to communicate anymore
      // assume that flows that reached max_duration have remaining of 0
      XBT_DEBUG("Action %p finished", action);
      action->finish(Action::State::FINISHED);
      get_action_heap().remove(action);
    }
  }
}

void NetworkCm02Model::update_actions_state_full(double now, double delta)
{
  for (auto it = std::begin(*get_started_action_set()); it != std::end(*get_started_action_set());) {
    NetworkCm02Action& action = static_cast<NetworkCm02Action&>(*it);
    ++it; // increment iterator here since the following calls to action.finish() may invalidate it
    XBT_DEBUG("Something happened to action %p", &action);
    double deltap = delta;
    if (action.latency_ > 0) {
      if (action.latency_ > deltap) {
        double_update(&action.latency_, deltap, sg_surf_precision);
        deltap = 0.0;
      } else {
        double_update(&deltap, action.latency_, sg_surf_precision);
        action.latency_ = 0.0;
      }
      if (action.latency_ <= 0.0 && not action.is_suspended())
        get_maxmin_system()->update_variable_weight(action.get_variable(), action.weight_);
    }

    if (not action.get_variable()->get_number_of_constraint()) {
      /* There is actually no link used, hence an infinite bandwidth. This happens often when using models like
       * vivaldi. In such case, just make sure that the action completes immediately.
       */
      action.update_remains(action.get_remains());
    }
    action.update_remains(action.get_variable()->get_value() * delta);

    if (action.get_max_duration() > NO_MAX_DURATION)
      action.update_max_duration(delta);

    if (((action.get_remains() <= 0) && (action.get_variable()->get_weight() > 0)) ||
        ((action.get_max_duration() > NO_MAX_DURATION) && (action.get_max_duration() <= 0))) {
      action.finish(Action::State::FINISHED);
    }
  }
}

Action* NetworkCm02Model::communicate(s4u::Host* src, s4u::Host* dst, double size, double rate)
{
  double latency = 0.0;
  std::vector<LinkImpl*> back_route;
  std::vector<LinkImpl*> route;

  XBT_IN("(%s,%s,%g,%g)", src->get_cname(), dst->get_cname(), size, rate);

  src->route_to(dst, route, &latency);
  xbt_assert(not route.empty() || latency,
             "You're trying to send data from %s to %s but there is no connecting path between these two hosts.",
             src->get_cname(), dst->get_cname());

  bool failed = std::any_of(route.begin(), route.end(), [](LinkImpl* link) { return link->is_off(); });

  if (cfg_crosstraffic) {
    dst->route_to(src, back_route, nullptr);
    if (not failed)
      failed = std::any_of(back_route.begin(), back_route.end(), [](LinkImpl* const& link) { return link->is_off(); });
  }

  NetworkCm02Action *action = new NetworkCm02Action(this, size, failed);
  action->weight_ = latency;
  action->latency_ = latency;
  action->rate_ = rate;
  if (get_update_algorithm() == Model::UpdateAlgo::LAZY) {
    action->set_last_update();
  }

  if (sg_weight_S_parameter > 0) {
    action->weight_ =
        std::accumulate(route.begin(), route.end(), action->weight_, [](double total, LinkImpl* const& link) {
          return total + sg_weight_S_parameter / link->get_bandwidth();
        });
  }

  double bandwidth_bound = route.empty() ? -1.0 : get_bandwidth_factor(size) * route.front()->get_bandwidth();

  for (auto const& link : route)
    bandwidth_bound = std::min(bandwidth_bound, get_bandwidth_factor(size) * link->get_bandwidth());

  action->lat_current_ = action->latency_;
  action->latency_ *= get_latency_factor(size);
  action->rate_ = get_bandwidth_constraint(action->rate_, bandwidth_bound, size);

  int constraints_per_variable = route.size();
  constraints_per_variable += back_route.size();

  if (action->latency_ > 0) {
    action->set_variable(get_maxmin_system()->variable_new(action, 0.0, -1.0, constraints_per_variable));
    if (get_update_algorithm() == Model::UpdateAlgo::LAZY) {
      // add to the heap the event when the latency is payed
      double date = action->latency_ + action->get_last_update();

      ActionHeap::Type type = route.empty() ? ActionHeap::Type::normal : ActionHeap::Type::latency;

      XBT_DEBUG("Added action (%p) one latency event at date %f", action, date);
      get_action_heap().insert(action, date, type);
    }
  } else
    action->set_variable(get_maxmin_system()->variable_new(action, 1.0, -1.0, constraints_per_variable));

  if (action->rate_ < 0) {
    get_maxmin_system()->update_variable_bound(
        action->get_variable(), (action->lat_current_ > 0) ? cfg_tcp_gamma / (2.0 * action->lat_current_) : -1.0);
  } else {
    get_maxmin_system()->update_variable_bound(
        action->get_variable(), (action->lat_current_ > 0)
                                    ? std::min(action->rate_, cfg_tcp_gamma / (2.0 * action->lat_current_))
                                    : action->rate_);
  }

  for (auto const& link : route)
    get_maxmin_system()->expand(link->get_constraint(), action->get_variable(), 1.0);

  if (cfg_crosstraffic) {
    XBT_DEBUG("Crosstraffic active: adding backward flow using 5%% of the available bandwidth");
    for (auto const& link : back_route)
      get_maxmin_system()->expand(link->get_constraint(), action->get_variable(), .05);

    // Change concurrency_share here, if you want that cross-traffic is included in the SURF concurrency
    // (You would also have to change simgrid::kernel::lmm::Element::get_concurrency())
    // action->getVariable()->set_concurrency_share(2)
  }
  XBT_OUT();

  simgrid::s4u::Link::on_communicate(action, src, dst);
  return action;
}

/************
 * Resource *
 ************/
NetworkCm02Link::NetworkCm02Link(NetworkCm02Model* model, const std::string& name, double bandwidth, double latency,
                                 s4u::Link::SharingPolicy policy, kernel::lmm::System* system)
    : LinkImpl(model, name, system->constraint_new(this, sg_bandwidth_factor * bandwidth))
{
  bandwidth_.scale = 1.0;
  bandwidth_.peak  = bandwidth;

  latency_.scale = 1.0;
  latency_.peak  = latency;

  if (policy == s4u::Link::SharingPolicy::FATPIPE)
    get_constraint()->unshare();

  simgrid::s4u::Link::on_creation(this->piface_);
}

void NetworkCm02Link::apply_event(tmgr_trace_event_t triggered, double value)
{
  /* Find out which of my iterators was triggered, and react accordingly */
  if (triggered == bandwidth_.event) {
    set_bandwidth(value);
    tmgr_trace_event_unref(&bandwidth_.event);

  } else if (triggered == latency_.event) {
    set_latency(value);
    tmgr_trace_event_unref(&latency_.event);

  } else if (triggered == state_event_) {
    if (value > 0)
      turn_on();
    else {
      kernel::lmm::Variable* var = nullptr;
      const kernel::lmm::Element* elem = nullptr;
      double now               = surf_get_clock();

      turn_off();
      while ((var = get_constraint()->get_variable(&elem))) {
        Action* action = static_cast<Action*>(var->get_id());

        if (action->get_state() == Action::State::INITED || action->get_state() == Action::State::STARTED) {
          action->set_finish_time(now);
          action->set_state(Action::State::FAILED);
        }
      }
    }
    tmgr_trace_event_unref(&state_event_);
  } else {
    xbt_die("Unknown event!\n");
  }

  XBT_DEBUG("There was a resource state event, need to update actions related to the constraint (%p)",
            get_constraint());
}

void NetworkCm02Link::set_bandwidth(double value)
{
  bandwidth_.peak = value;

  get_model()->get_maxmin_system()->update_constraint_bound(get_constraint(),
                                                            sg_bandwidth_factor * (bandwidth_.peak * bandwidth_.scale));

  LinkImpl::on_bandwidth_change();

  if (sg_weight_S_parameter > 0) {
    double delta = sg_weight_S_parameter / value - sg_weight_S_parameter / (bandwidth_.peak * bandwidth_.scale);

    kernel::lmm::Variable* var;
    const kernel::lmm::Element* elem     = nullptr;
    const kernel::lmm::Element* nextelem = nullptr;
    int numelem                  = 0;
    while ((var = get_constraint()->get_variable_safe(&elem, &nextelem, &numelem))) {
      NetworkCm02Action* action = static_cast<NetworkCm02Action*>(var->get_id());
      action->weight_ += delta;
      if (not action->is_suspended())
        get_model()->get_maxmin_system()->update_variable_weight(action->get_variable(), action->weight_);
    }
  }
}

void NetworkCm02Link::set_latency(double value)
{
  double delta                 = value - latency_.peak;
  kernel::lmm::Variable* var   = nullptr;
  const kernel::lmm::Element* elem     = nullptr;
  const kernel::lmm::Element* nextelem = nullptr;
  int numelem                  = 0;

  latency_.peak = value;

  while ((var = get_constraint()->get_variable_safe(&elem, &nextelem, &numelem))) {
    NetworkCm02Action* action = static_cast<NetworkCm02Action*>(var->get_id());
    action->lat_current_ += delta;
    action->weight_ += delta;
    if (action->rate_ < 0)
      get_model()->get_maxmin_system()->update_variable_bound(action->get_variable(), NetworkModel::cfg_tcp_gamma /
                                                                                          (2.0 * action->lat_current_));
    else {
      get_model()->get_maxmin_system()->update_variable_bound(
          action->get_variable(), std::min(action->rate_, NetworkModel::cfg_tcp_gamma / (2.0 * action->lat_current_)));

      if (action->rate_ < NetworkModel::cfg_tcp_gamma / (2.0 * action->lat_current_)) {
        XBT_INFO("Flow is limited BYBANDWIDTH");
      } else {
        XBT_INFO("Flow is limited BYLATENCY, latency of flow is %f", action->lat_current_);
      }
    }
    if (not action->is_suspended())
      get_model()->get_maxmin_system()->update_variable_weight(action->get_variable(), action->weight_);
  }
}

/**********
 * Action *
 **********/

void NetworkCm02Action::update_remains_lazy(double now)
{
  if (suspended_ != Action::SuspendStates::not_suspended)
    return;

  double delta        = now - get_last_update();
  double max_duration = get_max_duration();

  if (get_remains_no_update() > 0) {
    XBT_DEBUG("Updating action(%p): remains was %f, last_update was: %f", this, get_remains_no_update(),
              get_last_update());
    update_remains(get_last_value() * delta);

    XBT_DEBUG("Updating action(%p): remains is now %f", this, get_remains_no_update());
  }

  if (max_duration > NO_MAX_DURATION) {
    double_update(&max_duration, delta, sg_surf_precision);
    set_max_duration(max_duration);
  }

  if ((get_remains_no_update() <= 0 && (get_variable()->get_weight() > 0)) ||
      ((max_duration > NO_MAX_DURATION) && (max_duration <= 0))) {
    finish(Action::State::FINISHED);
    get_model()->get_action_heap().remove(this);
  }

  set_last_update();
  set_last_value(get_variable()->get_value());
}

}
}
} // namespace simgrid
