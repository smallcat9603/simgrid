/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "ptask_L07.hpp"
#include "surf/surf.hpp"
#include "xbt/config.hpp"

#include <unordered_set>

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(surf_host);
XBT_LOG_EXTERNAL_CATEGORY(xbt_cfg);

/**************************************/
/*** Resource Creation & Destruction **/
/**************************************/
void surf_host_model_init_ptask_L07()
{
  XBT_CINFO(xbt_cfg,"Switching to the L07 model to handle parallel tasks.");
  xbt_assert(not surf_cpu_model_pm, "Cannot switch to ptasks: CPU model already defined");
  xbt_assert(not surf_network_model, "Cannot switch to ptasks: network model already defined");

  surf_host_model = new simgrid::surf::HostL07Model();
  all_existing_models.push_back(surf_host_model);
}

namespace simgrid {
namespace surf {

HostL07Model::HostL07Model() : HostModel() {
  auto* maxmin_system = new simgrid::kernel::lmm::FairBottleneck(true /* selective update */);
  set_maxmin_system(maxmin_system);
  surf_network_model = new NetworkL07Model(this, maxmin_system);
  surf_cpu_model_pm  = new CpuL07Model(this, maxmin_system);
}

HostL07Model::~HostL07Model()
{
  delete surf_network_model;
  delete surf_cpu_model_pm;
}

CpuL07Model::CpuL07Model(HostL07Model* hmodel, kernel::lmm::System* sys)
    : CpuModel(Model::UpdateAlgo::FULL), hostModel_(hmodel)
{
  set_maxmin_system(sys);
}

CpuL07Model::~CpuL07Model()
{
  set_maxmin_system(nullptr);
}

NetworkL07Model::NetworkL07Model(HostL07Model* hmodel, kernel::lmm::System* sys)
    : NetworkModel(Model::UpdateAlgo::FULL), hostModel_(hmodel)
{
  set_maxmin_system(sys);
  loopback_ = NetworkL07Model::create_link("__loopback__", 498000000, 0.000015, s4u::Link::SharingPolicy::FATPIPE);
}

NetworkL07Model::~NetworkL07Model()
{
  set_maxmin_system(nullptr);
}

double HostL07Model::next_occuring_event(double now)
{
  double min = HostModel::next_occuring_event_full(now);
  for (kernel::resource::Action const& action : *get_started_action_set()) {
    const L07Action& net_action = static_cast<const L07Action&>(action);
    if (net_action.latency_ > 0 && (min < 0 || net_action.latency_ < min)) {
      min = net_action.latency_;
      XBT_DEBUG("Updating min with %p (start %f): %f", &net_action, net_action.get_start_time(), min);
    }
  }
  XBT_DEBUG("min value: %f", min);

  return min;
}

void HostL07Model::update_actions_state(double /*now*/, double delta)
{
  for (auto it = std::begin(*get_started_action_set()); it != std::end(*get_started_action_set());) {
    L07Action& action = static_cast<L07Action&>(*it);
    ++it; // increment iterator here since the following calls to action.finish() may invalidate it
    if (action.latency_ > 0) {
      if (action.latency_ > delta) {
        double_update(&(action.latency_), delta, sg_surf_precision);
      } else {
        action.latency_ = 0.0;
      }
      if ((action.latency_ <= 0.0) && (action.is_suspended() == 0)) {
        action.updateBound();
        get_maxmin_system()->update_variable_weight(action.get_variable(), 1.0);
        action.set_last_update();
      }
    }
    XBT_DEBUG("Action (%p) : remains (%g) updated by %g.", &action, action.get_remains(),
              action.get_variable()->get_value() * delta);
    action.update_remains(action.get_variable()->get_value() * delta);

    if (action.get_max_duration() > NO_MAX_DURATION)
      action.update_max_duration(delta);

    XBT_DEBUG("Action (%p) : remains (%g).", &action, action.get_remains());

    /* In the next if cascade, the action can be finished either because:
     *  - The amount of remaining work reached 0
     *  - The max duration was reached
     * If it's not done, it may have failed.
     */

    if (((action.get_remains() <= 0) && (action.get_variable()->get_weight() > 0)) ||
        ((action.get_max_duration() > NO_MAX_DURATION) && (action.get_max_duration() <= 0))) {
      action.finish(kernel::resource::Action::State::FINISHED);
    } else {
      /* Need to check that none of the model has failed */
      int i = 0;
      kernel::lmm::Constraint* cnst = action.get_variable()->get_constraint(i);
      while (cnst != nullptr) {
        i++;
        void* constraint_id = cnst->get_id();
        if (static_cast<simgrid::kernel::resource::Resource*>(constraint_id)->is_off()) {
          XBT_DEBUG("Action (%p) Failed!!", &action);
          action.finish(kernel::resource::Action::State::FAILED);
          break;
        }
        cnst = action.get_variable()->get_constraint(i);
      }
    }
  }
}

kernel::resource::Action* HostL07Model::execute_parallel(int host_nb, sg_host_t* host_list, double* flops_amount,
                                                         double* bytes_amount, double rate)
{
  return new L07Action(this, host_nb, host_list, flops_amount, bytes_amount, rate);
}

L07Action::L07Action(kernel::resource::Model* model, int host_nb, sg_host_t* host_list, double* flops_amount,
                     double* bytes_amount, double rate)
    : CpuAction(model, 1, 0), computationAmount_(flops_amount), communicationAmount_(bytes_amount), rate_(rate)
{
  int nb_link = 0;
  int nb_used_host = 0; /* Only the hosts with something to compute (>0 flops) are counted) */
  double latency = 0.0;
  this->set_last_update();

  this->hostList_->reserve(host_nb);
  for (int i = 0; i < host_nb; i++) {
    this->hostList_->push_back(host_list[i]);
    if (flops_amount[i] > 0)
      nb_used_host++;
  }

  /* Compute the number of affected resources... */
  if(bytes_amount != nullptr) {
    std::unordered_set<const char*> affected_links;

    for (int i = 0; i < host_nb; i++) {
      for (int j = 0; j < host_nb; j++) {

        if (bytes_amount[i * host_nb + j] > 0) {
          double lat=0.0;

          std::vector<kernel::resource::LinkImpl*> route;
          hostList_->at(i)->route_to(hostList_->at(j), route, &lat);
          latency = std::max(latency, lat);

          for (auto const& link : route)
            affected_links.insert(link->get_cname());
        }
      }
    }

    nb_link = affected_links.size();
  }

  XBT_DEBUG("Creating a parallel task (%p) with %d hosts and %d unique links.", this, host_nb, nb_link);
  latency_ = latency;

  set_variable(model->get_maxmin_system()->variable_new(this, 1.0, (rate > 0 ? rate : -1.0), host_nb + nb_link));

  if (latency_ > 0)
    model->get_maxmin_system()->update_variable_weight(get_variable(), 0.0);

  for (int i = 0; i < host_nb; i++)
    model->get_maxmin_system()->expand(host_list[i]->pimpl_cpu->get_constraint(), get_variable(), flops_amount[i]);

  if(bytes_amount != nullptr) {
    for (int i = 0; i < host_nb; i++) {
      for (int j = 0; j < host_nb; j++) {
        if (bytes_amount[i * host_nb + j] > 0.0) {
          std::vector<kernel::resource::LinkImpl*> route;
          hostList_->at(i)->route_to(hostList_->at(j), route, nullptr);

          for (auto const& link : route)
            model->get_maxmin_system()->expand_add(link->get_constraint(), this->get_variable(),
                                                   bytes_amount[i * host_nb + j]);
        }
      }
    }
  }

  if (nb_link + nb_used_host == 0) {
    this->set_cost(1.0);
    this->set_remains(0.0);
  }
  delete[] host_list;
}

kernel::resource::Action* NetworkL07Model::communicate(s4u::Host* src, s4u::Host* dst, double size, double rate)
{
  sg_host_t* host_list = new sg_host_t[2]();
  double* flops_amount = new double[2]();
  double* bytes_amount = new double[4]();

  host_list[0]    = src;
  host_list[1]    = dst;
  bytes_amount[1] = size;

  return hostModel_->execute_parallel(2, host_list, flops_amount, bytes_amount, rate);
}

Cpu* CpuL07Model::create_cpu(simgrid::s4u::Host* host, std::vector<double>* speed_per_pstate, int core)
{
  return new CpuL07(this, host, speed_per_pstate, core);
}

kernel::resource::LinkImpl* NetworkL07Model::create_link(const std::string& name, double bandwidth, double latency,
                                                         s4u::Link::SharingPolicy policy)
{
  return new LinkL07(this, name, bandwidth, latency, policy);
}

/************
 * Resource *
 ************/

CpuL07::CpuL07(CpuL07Model* model, simgrid::s4u::Host* host, std::vector<double>* speed_per_pstate, int core)
    : Cpu(model, host, model->get_maxmin_system()->constraint_new(this, speed_per_pstate->front()), speed_per_pstate,
          core)
{
}

CpuL07::~CpuL07()=default;

LinkL07::LinkL07(NetworkL07Model* model, const std::string& name, double bandwidth, double latency,
                 s4u::Link::SharingPolicy policy)
    : LinkImpl(model, name, model->get_maxmin_system()->constraint_new(this, bandwidth))
{
  bandwidth_.peak = bandwidth;
  latency_.peak   = latency;

  if (policy == s4u::Link::SharingPolicy::FATPIPE)
    get_constraint()->unshare();

  s4u::Link::on_creation(this->piface_);
}

kernel::resource::Action* CpuL07::execution_start(double size)
{
  sg_host_t* host_list = new sg_host_t[1]();
  double* flops_amount = new double[1]();

  host_list[0]    = get_host();
  flops_amount[0] = size;

  return static_cast<CpuL07Model*>(get_model())->hostModel_->execute_parallel(1, host_list, flops_amount, nullptr, -1);
}

kernel::resource::Action* CpuL07::sleep(double duration)
{
  L07Action *action = static_cast<L07Action*>(execution_start(1.0));
  action->set_max_duration(duration);
  action->suspended_ = kernel::resource::Action::SuspendStates::sleeping;
  get_model()->get_maxmin_system()->update_variable_weight(action->get_variable(), 0.0);

  return action;
}

bool CpuL07::is_used()
{
  return get_model()->get_maxmin_system()->constraint_used(get_constraint());
}

/** @brief take into account changes of speed (either load or max) */
void CpuL07::on_speed_change()
{
  kernel::lmm::Variable* var = nullptr;
  const kernel::lmm::Element* elem = nullptr;

  get_model()->get_maxmin_system()->update_constraint_bound(get_constraint(), speed_.peak * speed_.scale);
  while ((var = get_constraint()->get_variable(&elem))) {
    kernel::resource::Action* action = static_cast<kernel::resource::Action*>(var->get_id());

    get_model()->get_maxmin_system()->update_variable_bound(action->get_variable(), speed_.scale * speed_.peak);
  }

  Cpu::on_speed_change();
}

bool LinkL07::is_used()
{
  return get_model()->get_maxmin_system()->constraint_used(get_constraint());
}

void CpuL07::apply_event(tmgr_trace_event_t triggered, double value)
{
  XBT_DEBUG("Updating cpu %s (%p) with value %g", get_cname(), this, value);
  if (triggered == speed_.event) {
    speed_.scale = value;
    on_speed_change();
    tmgr_trace_event_unref(&speed_.event);

  } else if (triggered == state_event_) {
    if (value > 0) {
      if (is_off()) {
        XBT_VERB("Restart processes on host %s", get_host()->get_cname());
        get_host()->turn_on();
      }
    } else
      get_host()->turn_off();
    tmgr_trace_event_unref(&state_event_);

  } else {
    xbt_die("Unknown event!\n");
  }
}

void LinkL07::apply_event(tmgr_trace_event_t triggered, double value)
{
  XBT_DEBUG("Updating link %s (%p) with value=%f", get_cname(), this, value);
  if (triggered == bandwidth_.event) {
    set_bandwidth(value);
    tmgr_trace_event_unref(&bandwidth_.event);

  } else if (triggered == latency_.event) {
    set_latency(value);
    tmgr_trace_event_unref(&latency_.event);

  } else if (triggered == state_event_) {
    if (value > 0)
      turn_on();
    else
      turn_off();
    tmgr_trace_event_unref(&state_event_);

  } else {
    xbt_die("Unknown event ! \n");
  }
}

void LinkL07::set_bandwidth(double value)
{
  bandwidth_.peak = value;
  LinkImpl::on_bandwidth_change();

  get_model()->get_maxmin_system()->update_constraint_bound(get_constraint(), bandwidth_.peak * bandwidth_.scale);
}

void LinkL07::set_latency(double value)
{
  kernel::lmm::Variable* var = nullptr;
  L07Action *action;
  const kernel::lmm::Element* elem = nullptr;

  latency_.peak = value;
  while ((var = get_constraint()->get_variable(&elem))) {
    action = static_cast<L07Action*>(var->get_id());
    action->updateBound();
  }
}
LinkL07::~LinkL07() = default;

/**********
 * Action *
 **********/

L07Action::~L07Action(){
  delete hostList_;
  delete[] communicationAmount_;
  delete[] computationAmount_;
}

void L07Action::updateBound()
{
  double lat_current = 0.0;

  int hostNb = hostList_->size();

  if (communicationAmount_ != nullptr) {
    for (int i = 0; i < hostNb; i++) {
      for (int j = 0; j < hostNb; j++) {

        if (communicationAmount_[i * hostNb + j] > 0) {
          double lat = 0.0;
          std::vector<kernel::resource::LinkImpl*> route;
          hostList_->at(i)->route_to(hostList_->at(j), route, &lat);

          lat_current = std::max(lat_current, lat * communicationAmount_[i * hostNb + j]);
        }
      }
    }
  }
  double lat_bound = kernel::resource::NetworkModel::cfg_tcp_gamma / (2.0 * lat_current);
  XBT_DEBUG("action (%p) : lat_bound = %g", this, lat_bound);
  if ((latency_ <= 0.0) && (suspended_ == Action::SuspendStates::not_suspended)) {
    if (rate_ < 0)
      get_model()->get_maxmin_system()->update_variable_bound(get_variable(), lat_bound);
    else
      get_model()->get_maxmin_system()->update_variable_bound(get_variable(), std::min(rate_, lat_bound));
  }
}

}
}
