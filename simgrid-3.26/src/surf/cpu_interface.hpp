/* Copyright (c) 2004-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SURF_CPU_INTERFACE_HPP_
#define SURF_CPU_INTERFACE_HPP_

#include "simgrid/kernel/resource/Model.hpp"
#include "simgrid/kernel/resource/Resource.hpp"
#include "simgrid/s4u/Host.hpp"
#include "src/kernel/lmm/maxmin.hpp"

#include <list>

/***********
 * Classes *
 ***********/

namespace simgrid {
namespace kernel {
namespace resource {

/** @ingroup SURF_cpu_interface
 * @brief SURF cpu model interface class
 * @details A model is an object which handle the interactions between its Resources and its Actions
 */
class XBT_PUBLIC CpuModel : public Model {
public:
  using Model::Model;

  /**
   * @brief Create a Cpu
   *
   * @param host The host that will have this CPU
   * @param speed_per_pstate Processor speed (in Flops) of each pstate.
   *                         This ignores any potential external load coming from a trace.
   * @param core The number of core of this Cpu
   */
  virtual Cpu* create_cpu(s4u::Host* host, const std::vector<double>& speed_per_pstate, int core) = 0;

  void update_actions_state_lazy(double now, double delta) override;
  void update_actions_state_full(double now, double delta) override;
};

/************
 * Resource *
 ************/

class CpuAction;

/** @ingroup SURF_cpu_interface
* @brief SURF cpu resource interface class
* @details A Cpu represent a cpu associated to a host
*/
class XBT_PUBLIC Cpu : public Resource {
  int core_count_ = 1;
  s4u::Host* host_;
  int pstate_ = 0;                       /*< Current pstate (index in the speed_per_pstate_)*/
  std::vector<double> speed_per_pstate_; /*< List of supported CPU capacities (pstate related). Not 'const' because VCPU
                                            get modified on migration */
  friend simgrid::vm::VirtualMachineImpl; // Resets the VCPU

public:
  /**
   * @brief Cpu constructor
   *
   * @param model The CpuModel associated to this Cpu
   * @param host The host in which this Cpu should be plugged
   * @param constraint The lmm constraint associated to this Cpu if it is part of a LMM component
   * @param speedPerPstate Processor speed (in flop per second) for each pstate
   * @param core The number of core of this Cpu
   */
  Cpu(Model* model, s4u::Host* host, lmm::Constraint* constraint, const std::vector<double>& speed_per_pstate,
      int core);

  /**
   * @brief Cpu constructor
   *
   * @param model The CpuModel associated to this Cpu
   * @param host The host in which this Cpu should be plugged
   * @param speedPerPstate Processor speed (in flop per second) for each pstate
   * @param core The number of core of this Cpu
   */
  Cpu(Model* model, s4u::Host* host, const std::vector<double>& speed_per_pstate, int core);

  Cpu(const Cpu&) = delete;
  Cpu& operator=(const Cpu&) = delete;

  /**
   * @brief Execute some quantity of computation
   *
   * @param size The value of the processing amount (in flop) needed to process
   * @return The CpuAction corresponding to the processing
   */
  virtual CpuAction* execution_start(double size) = 0;

  /**
   * @brief Execute some quantity of computation on more than one core
   *
   * @param size The value of the processing amount (in flop) needed to process
   * @param requested_cores The desired amount of cores. Must be >= 1
   * @return The CpuAction corresponding to the processing
   */
  virtual CpuAction* execution_start(double size, int requested_cores) = 0;

  /**
   * @brief Make a process sleep for duration (in seconds)
   *
   * @param duration The number of seconds to sleep
   * @return The CpuAction corresponding to the sleeping
   */
  virtual CpuAction* sleep(double duration) = 0;

  /** @brief Get the amount of cores */
  virtual int get_core_count();

  /** @brief Get a forecast of the speed (in flops/s) if the load were as provided.
   *
   * The provided load should encompasses both the application's activities and the external load that come from a trace.
   *
   * Use a load of 1.0 to compute the amount of flops that the Cpu would deliver with one CPU-bound task.
   * If you use a load of 0, this function will return 0: when nobody is using the Cpu, it delivers nothing.
   *
   * If you want to know the amount of flops currently delivered, use  load = get_load()*get_speed_ratio()
   */
  virtual double get_speed(double load) const;

protected:
  /** @brief Take speed changes (either load or max) into account */
  virtual void on_speed_change();

  /** Reset most characteristics of this CPU to the one of that CPU.
   *
   * Used to reset a VCPU when its VM migrates to another host, so it only resets the fields that should be in this
   *case.
   **/
  virtual void reset_vcpu(Cpu* that);

public:
  /** @brief Get the available speed ratio, between 0 and 1.
   *
   * This accounts for external load (see @ref set_speed_trace()).
   */
  virtual double get_speed_ratio();

  /** @brief Get the peak processor speed (in flops/s), at the specified pstate */
  virtual double get_pstate_peak_speed(int pstate_index) const;

  virtual int get_pstate_count() const;
  virtual void set_pstate(int pstate_index);
  virtual int get_pstate() const;

  s4u::Host* get_host() { return host_; }

  /*< @brief Setup the trace file with availability events (peak speed changes due to external load).
   * Trace must contain relative values (ratio between 0 and 1)
   */
  virtual void set_speed_profile(profile::Profile* profile);

protected:
  Metric speed_                  = {1.0, 0, nullptr};
};

/**********
 * Action *
 **********/

 /** @ingroup SURF_cpu_interface
 * @brief A CpuAction represents the execution of code on one or several Cpus
 */
class XBT_PUBLIC CpuAction : public Action {
public:
  /** @brief Signal emitted when the action state changes (ready/running/done, etc)
   *  Signature: `void(CpuAction const& action, simgrid::kernel::resource::Action::State previous)`
   */
  static xbt::signal<void(CpuAction const&, Action::State)> on_state_change;

  using Action::Action;

  void set_state(Action::State state) override;

  void update_remains_lazy(double now) override;
  std::list<Cpu*> cpus() const;

  void suspend() override;
  void resume() override;
};
} // namespace resource
} // namespace kernel
} // namespace simgrid

#endif /* SURF_CPU_INTERFACE_HPP_ */
