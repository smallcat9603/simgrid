/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "workstation_clm03.hpp"
#include "vm_workstation_interface.hpp"
#include "cpu_cas01.hpp"
#include "simgrid/sg_config.h"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(surf_workstation);

/*************
 * CallBacks *
 *************/

/*********
 * Model *
 *********/

void surf_workstation_model_init_current_default(void)
{
  surf_workstation_model = new WorkstationCLM03Model();
  xbt_cfg_setdefault_boolean(_sg_cfg_set, "network/crosstraffic", "yes");
  surf_cpu_model_init_Cas01();
  surf_network_model_init_LegrandVelho();
  surf_workstation_model->p_cpuModel = surf_cpu_model_pm;

  ModelPtr model = surf_workstation_model;
  xbt_dynar_push(model_list, &model);
  xbt_dynar_push(model_list_invoke, &model);
  sg_platf_host_add_cb(workstation_parse_init);
}

void surf_workstation_model_init_compound()
{

  xbt_assert(surf_cpu_model_pm, "No CPU model defined yet!");
  xbt_assert(surf_network_model, "No network model defined yet!");
  surf_workstation_model = new WorkstationCLM03Model();

  ModelPtr model = surf_workstation_model;
  xbt_dynar_push(model_list, &model);
  xbt_dynar_push(model_list_invoke, &model);
  sg_platf_host_add_cb(workstation_parse_init);
}

WorkstationCLM03Model::WorkstationCLM03Model()
 : WorkstationModel("Workstation")
{
}

WorkstationCLM03Model::~WorkstationCLM03Model()
{}

WorkstationPtr WorkstationCLM03Model::createWorkstation(const char *name){
  WorkstationPtr workstation = new WorkstationCLM03(surf_workstation_model, name, NULL,
		  (xbt_dynar_t)xbt_lib_get_or_null(storage_lib, name, ROUTING_STORAGE_HOST_LEVEL),
		  (RoutingEdgePtr)xbt_lib_get_or_null(host_lib, name, ROUTING_HOST_LEVEL),
		  static_cast<CpuPtr>(xbt_lib_get_or_null(host_lib, name, SURF_CPU_LEVEL)));
  XBT_DEBUG("Create workstation %s with %ld mounted disks", name, xbt_dynar_length(workstation->p_storage));
  xbt_lib_set(host_lib, name, SURF_WKS_LEVEL, workstation);
  return workstation;
}

double WorkstationCLM03Model::shareResources(double now){
  adjustWeightOfDummyCpuActions();

  double min_by_cpu = p_cpuModel->shareResources(now);
  double min_by_net = (strcmp(surf_network_model->getName(), "network NS3")) ? surf_network_model->shareResources(now) : -1;
  double min_by_sto = -1;
  if (p_cpuModel == surf_cpu_model_pm)
	min_by_sto = surf_storage_model->shareResources(now);

  XBT_DEBUG("model %p, %s min_by_cpu %f, %s min_by_net %f, %s min_by_sto %f",
      this, surf_cpu_model_pm->getName(), min_by_cpu,
            surf_network_model->getName(), min_by_net,
            surf_storage_model->getName(), min_by_sto);

  double res = max(max(min_by_cpu, min_by_net), min_by_sto);
  if (min_by_cpu >= 0.0 && min_by_cpu < res)
	res = min_by_cpu;
  if (min_by_net >= 0.0 && min_by_net < res)
	res = min_by_net;
  if (min_by_sto >= 0.0 && min_by_sto < res)
	res = min_by_sto;
  return res;
}

void WorkstationCLM03Model::updateActionsState(double /*now*/, double /*delta*/){
  return;
}

ActionPtr WorkstationCLM03Model::executeParallelTask(int workstation_nb,
                                        void **workstation_list,
                                        double *computation_amount,
                                        double *communication_amount,
                                        double rate){
#define cost_or_zero(array,pos) ((array)?(array)[pos]:0.0)
  ActionPtr action =NULL;
  if ((workstation_nb == 1)
      && (cost_or_zero(communication_amount, 0) == 0.0)){
    action = ((WorkstationCLM03Ptr)workstation_list[0])->execute(computation_amount[0]);
  } else if ((workstation_nb == 1)
           && (cost_or_zero(computation_amount, 0) == 0.0)) {
    action = communicate((WorkstationCLM03Ptr)workstation_list[0],
        (WorkstationCLM03Ptr)workstation_list[0],communication_amount[0], rate);
  } else if ((workstation_nb == 2)
             && (cost_or_zero(computation_amount, 0) == 0.0)
             && (cost_or_zero(computation_amount, 1) == 0.0)) {
    int i,nb = 0;
    double value = 0.0;

    for (i = 0; i < workstation_nb * workstation_nb; i++) {
      if (cost_or_zero(communication_amount, i) > 0.0) {
        nb++;
        value = cost_or_zero(communication_amount, i);
      }
    }
    if (nb == 1){
      action = communicate((WorkstationCLM03Ptr)workstation_list[0],
          (WorkstationCLM03Ptr)workstation_list[1],value, rate);
    }
  } else
    THROW_UNIMPLEMENTED;      /* This model does not implement parallel tasks */
#undef cost_or_zero
  xbt_free((WorkstationCLM03Ptr)workstation_list);
  return action;
}

ActionPtr WorkstationCLM03Model::communicate(WorkstationPtr src, WorkstationPtr dst, double size, double rate){
  return surf_network_model->communicate(src->p_netElm, dst->p_netElm, size, rate);
}



/************
 * Resource *
 ************/
WorkstationCLM03::WorkstationCLM03(WorkstationModelPtr model, const char* name, xbt_dict_t properties, xbt_dynar_t storage, RoutingEdgePtr netElm, CpuPtr cpu)
  : Workstation(model, name, properties, storage, netElm, cpu) {}

bool WorkstationCLM03::isUsed(){
  THROW_IMPOSSIBLE;             /* This model does not implement parallel tasks */
  return -1;
}

void WorkstationCLM03::updateState(tmgr_trace_event_t /*event_type*/, double /*value*/, double /*date*/){
  THROW_IMPOSSIBLE;             /* This model does not implement parallel tasks */
}

ActionPtr WorkstationCLM03::execute(double size) {
  return p_cpu->execute(size);
}

ActionPtr WorkstationCLM03::sleep(double duration) {
  return p_cpu->sleep(duration);
}

e_surf_resource_state_t WorkstationCLM03::getState() {
  return p_cpu->getState();
}

/**********
 * Action *
 **********/
