/* Copyright (c) 2013-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "cpu_interface.hpp"
#include "src/surf/surf_interface.hpp"
#include "surf/surf.hpp"

XBT_LOG_EXTERNAL_CATEGORY(surf_kernel);
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(surf_cpu, surf, "Logging specific to the SURF cpu module");

simgrid::surf::CpuModel *surf_cpu_model_pm;
simgrid::surf::CpuModel *surf_cpu_model_vm;

namespace simgrid {
namespace surf {

/*********
 * Model *
 *********/

void CpuModel::update_actions_state_lazy(double now, double /*delta*/)
{
  while (not get_action_heap().empty() && double_equals(get_action_heap().top_date(), now, sg_surf_precision)) {

    CpuAction* action = static_cast<CpuAction*>(get_action_heap().pop());
    XBT_CDEBUG(surf_kernel, "Something happened to action %p", action);

    action->finish(kernel::resource::Action::State::FINISHED);
    XBT_CDEBUG(surf_kernel, "Action %p finished", action);
  }
}

void CpuModel::update_actions_state_full(double now, double delta)
{
  for (auto it = std::begin(*get_started_action_set()); it != std::end(*get_started_action_set());) {
    CpuAction& action = static_cast<CpuAction&>(*it);
    ++it; // increment iterator here since the following calls to action.finish() may invalidate it

    action.update_remains(action.get_variable()->get_value() * delta);

    if (action.get_max_duration() != NO_MAX_DURATION)
      action.update_max_duration(delta);

    if (((action.get_remains_no_update() <= 0) && (action.get_variable()->get_weight() > 0)) ||
        ((action.get_max_duration() != NO_MAX_DURATION) && (action.get_max_duration() <= 0))) {
      action.finish(kernel::resource::Action::State::FINISHED);
    }
  }
}

/************
 * Resource *
 ************/
Cpu::Cpu(kernel::resource::Model* model, simgrid::s4u::Host* host, std::vector<double>* speedPerPstate, int core)
    : Cpu(model, host, nullptr /*constraint*/, speedPerPstate, core)
{
}

Cpu::Cpu(kernel::resource::Model* model, simgrid::s4u::Host* host, kernel::lmm::Constraint* constraint,
         std::vector<double>* speedPerPstate, int core)
    : Resource(model, host->get_cname(), constraint), core_count_(core), host_(host)
{
  xbt_assert(core > 0, "Host %s must have at least one core, not 0.", host->get_cname());

  speed_.peak = speedPerPstate->front();
  speed_.scale = 1;
  host->pimpl_cpu = this;
  xbt_assert(speed_.scale > 0, "Speed of host %s must be >0", host->get_cname());

  // Copy the power peak array:
  for (double const& value : *speedPerPstate) {
    speed_per_pstate_.push_back(value);
  }
}

Cpu::~Cpu()
{
  if (get_model() == surf_cpu_model_pm)
    speed_per_pstate_.clear();
}

int Cpu::get_pstate_count()
{
  return speed_per_pstate_.size();
}

void Cpu::set_pstate(int pstate_index)
{
  xbt_assert(pstate_index <= static_cast<int>(speed_per_pstate_.size()),
             "Invalid parameters for CPU %s (pstate %d > length of pstates %d). Please fix your platform file, or your "
             "call to change the pstate.",
             get_cname(), pstate_index, static_cast<int>(speed_per_pstate_.size()));

  double new_peak_speed = speed_per_pstate_[pstate_index];
  pstate_ = pstate_index;
  speed_.peak = new_peak_speed;

  on_speed_change();
}

int Cpu::get_pstate()
{
  return pstate_;
}

double Cpu::get_pstate_peak_speed(int pstate_index)
{
  xbt_assert((pstate_index <= static_cast<int>(speed_per_pstate_.size())),
             "Invalid parameters (pstate index out of bounds)");

  return speed_per_pstate_[pstate_index];
}

double Cpu::get_speed(double load)
{
  return load * speed_.peak;
}

double Cpu::get_speed_ratio()
{
/* number between 0 and 1 */
  return speed_.scale;
}

void Cpu::on_speed_change()
{
  s4u::Host::on_speed_change(*host_);
}

int Cpu::get_core_count()
{
  return core_count_;
}

void Cpu::set_speed_trace(tmgr_trace_t trace)
{
  xbt_assert(speed_.event == nullptr, "Cannot set a second speed trace to Host %s", host_->get_cname());

  speed_.event = future_evt_set.add_trace(trace, this);
}


/**********
 * Action *
 **********/

void CpuAction::update_remains_lazy(double now)
{
  xbt_assert(get_state_set() == get_model()->get_started_action_set(),
             "You're updating an action that is not running.");
  xbt_assert(get_priority() > 0, "You're updating an action that seems suspended.");

  double delta = now - get_last_update();

  if (get_remains_no_update() > 0) {
    XBT_CDEBUG(surf_kernel, "Updating action(%p): remains was %f, last_update was: %f", this, get_remains_no_update(),
               get_last_update());
    update_remains(get_last_value() * delta);

    XBT_CDEBUG(surf_kernel, "Updating action(%p): remains is now %f", this, get_remains_no_update());
  }

  set_last_update();
  set_last_value(get_variable()->get_value());
}

simgrid::xbt::signal<void(simgrid::surf::CpuAction*, kernel::resource::Action::State)> CpuAction::on_state_change;

void CpuAction::suspend(){
  Action::State previous = get_state();
  on_state_change(this, previous);
  Action::suspend();
}

void CpuAction::resume(){
  Action::State previous = get_state();
  on_state_change(this, previous);
  Action::resume();
}

void CpuAction::set_state(Action::State state)
{
  Action::State previous = get_state();
  Action::set_state(state);
  on_state_change(this, previous);
}

/** @brief returns a list of all CPUs that this action is using */
std::list<Cpu*> CpuAction::cpus() {
  std::list<Cpu*> retlist;
  int llen = get_variable()->get_number_of_constraint();

  for (int i = 0; i < llen; i++) {
    /* Beware of composite actions: ptasks put links and cpus together */
    // extra pb: we cannot dynamic_cast from void*...
    kernel::resource::Resource* resource =
        static_cast<kernel::resource::Resource*>(get_variable()->get_constraint(i)->get_id());
    Cpu* cpu           = dynamic_cast<Cpu*>(resource);
    if (cpu != nullptr)
      retlist.push_back(cpu);
  }

  return retlist;
}

}
}
