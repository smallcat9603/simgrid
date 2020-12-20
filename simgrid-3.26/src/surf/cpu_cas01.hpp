/* Copyright (c) 2013-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_SURF_CPUCAS01_HPP
#define SIMGRID_SURF_CPUCAS01_HPP

#include "cpu_interface.hpp"
#include "xbt/base.h"

/***********
 * Classes *
 ***********/

namespace simgrid {
namespace kernel {
namespace resource {

class XBT_PRIVATE CpuCas01Model;
class XBT_PRIVATE CpuCas01;
class XBT_PRIVATE CpuCas01Action;

/*********
 * Model *
 *********/

class CpuCas01Model : public CpuModel {
public:
  explicit CpuCas01Model(Model::UpdateAlgo algo);
  CpuCas01Model(const CpuCas01Model&) = delete;
  CpuCas01Model& operator=(const CpuCas01Model&) = delete;
  ~CpuCas01Model() override;

  Cpu* create_cpu(s4u::Host* host, const std::vector<double>& speed_per_pstate, int core) override;
};

/************
 * Resource *
 ************/

class CpuCas01 : public Cpu {
public:
  CpuCas01(CpuCas01Model* model, s4u::Host* host, const std::vector<double>& speed_per_pstate, int core);
  CpuCas01(const CpuCas01&) = delete;
  CpuCas01& operator=(const CpuCas01&) = delete;
  ~CpuCas01() override;
  void apply_event(profile::Event* event, double value) override;
  CpuAction* execution_start(double size) override;
  CpuAction* execution_start(double size, int requested_cores) override;
  CpuAction* sleep(double duration) override;

  bool is_used() const override;

protected:
  void on_speed_change() override;
};

/**********
 * Action *
 **********/
class CpuCas01Action : public CpuAction {
  friend CpuAction* CpuCas01::execution_start(double size);
  friend CpuAction* CpuCas01::sleep(double duration);

public:
  CpuCas01Action(Model* model, double cost, bool failed, double speed, lmm::Constraint* constraint, int core_count);
  CpuCas01Action(Model* model, double cost, bool failed, double speed, lmm::Constraint* constraint);
  CpuCas01Action(const CpuCas01Action&) = delete;
  CpuCas01Action& operator=(const CpuCas01Action&) = delete;
  ~CpuCas01Action() override;
  int requested_core() const;

private:
  int requested_core_ = 1;
};

} // namespace resource
} // namespace kernel
} // namespace simgrid

#endif
