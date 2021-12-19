/* Copyright (c) 2004-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SURF_MODEL_H_
#define SURF_MODEL_H_

#include <xbt.h>
#include <string>
#include <vector>
#include <memory>
#include <boost/function.hpp>
#include <boost/intrusive/list.hpp>
#include "surf/trace_mgr.h"
#include "xbt/lib.h"
#include "surf/surf_routing.h"
#include "simgrid/platf_interface.h"
#include "surf/surf.h"
#include "surf/surf_private.h"
#include "internal_config.h"

#ifdef LIBSIGC
#include <sigc++/sigc++.h>
#define surf_callback(arg1, ...)  sigc::signal<arg1,__VA_ARGS__>
#define surf_callback_connect(callback, fun_ptr) callback.connect(sigc::ptr_fun(fun_ptr))
#define surf_callback_emit(callback, ...) callback.emit(__VA_ARGS__)
#else
#include <boost/signals2.hpp>
#define surf_callback(arg1, ...)  boost::signals2::signal<arg1(__VA_ARGS__)>
#define surf_callback_connect(callback, fun_ptr) callback.connect(fun_ptr)
#define surf_callback_emit(callback, ...) callback(__VA_ARGS__)
#endif

extern tmgr_history_t history;
#define NO_MAX_DURATION -1.0

using namespace std;

/*********
 * Utils *
 *********/

/* user-visible parameters */
extern double sg_tcp_gamma;
extern double sg_sender_gap;
extern double sg_latency_factor;
extern double sg_bandwidth_factor;
extern double sg_weight_S_parameter;
extern int sg_network_crosstraffic;
#ifdef HAVE_GTNETS
extern double sg_gtnets_jitter;
extern int sg_gtnets_jitter_seed;
#endif
extern xbt_dynar_t surf_path;

extern "C" {
XBT_PUBLIC(double) surf_get_clock(void);
}

extern double sg_sender_gap;
XBT_PUBLIC(int)  SURF_CPU_LEVEL;    //Surf cpu level

extern surf_callback(void, void) surfExitCallbacks;

int __surf_is_absolute_file_path(const char *file_path);

/***********
 * Classes *
 ***********/
//class Model;
typedef Model* ModelPtr;

//class Resource;
typedef Resource* ResourcePtr;

//class Action;
typedef Action* ActionPtr;

typedef boost::intrusive::list<Action> ActionList;
typedef ActionList* ActionListPtr;
typedef boost::intrusive::list_base_hook<> actionHook;

struct lmmTag;
typedef boost::intrusive::list<Action, boost::intrusive::base_hook<boost::intrusive::list_base_hook<boost::intrusive::tag<lmmTag> > > > ActionLmmList;
typedef ActionLmmList* ActionLmmListPtr;
typedef boost::intrusive::list_base_hook<boost::intrusive::tag<lmmTag> > actionLmmHook;


enum heap_action_type{
  LATENCY = 100,
  MAX_DURATION,
  NORMAL,
  NOTSET
};

/*********
 * Trace *
 *********/
/* For the trace and trace:connect tag (store their content till the end of the parsing) */
XBT_PUBLIC_DATA(xbt_dict_t) traces_set_list;
XBT_PUBLIC_DATA(xbt_dict_t) trace_connect_list_host_avail;
XBT_PUBLIC_DATA(xbt_dict_t) trace_connect_list_power;
XBT_PUBLIC_DATA(xbt_dict_t) trace_connect_list_link_avail;
XBT_PUBLIC_DATA(xbt_dict_t) trace_connect_list_bandwidth;
XBT_PUBLIC_DATA(xbt_dict_t) trace_connect_list_latency;

/*********
 * Model *
 *********/
XBT_PUBLIC_DATA(xbt_dynar_t) model_list;

/** @ingroup SURF_interface
 * @brief SURF model interface class
 * @details A model is an object which handle the interactions between its Resources and its Actions
 */
XBT_PUBLIC_CLASS Model {
public:
  /**
   * @brief Model constructor
   *
   * @param name the name of the model
   */
  Model(const char *name);

  /**
   * @brief Model destructor
   */
  virtual ~Model();

  virtual void addTraces() =0;

  /**
   * @brief Get the name of the current Model
   *
   * @return The name of the current Model
   */
  const char *getName() {return p_name;}

  /**
   * @brief Get the set of [actions](@ref Action) in *ready* state
   *
   * @return The set of [actions](@ref Action) in *ready* state
   */
  virtual ActionListPtr getReadyActionSet() {return p_readyActionSet;}

  /**
   * @brief Get the set of [actions](@ref Action) in *running* state
   *
   * @return The set of [actions](@ref Action) in *running* state
   */
  virtual ActionListPtr getRunningActionSet() {return p_runningActionSet;}

  /**
   * @brief Get the set of [actions](@ref Action) in *failed* state
   *
   * @return The set of [actions](@ref Action) in *failed* state
   */
  virtual ActionListPtr getFailedActionSet() {return p_failedActionSet;}

  /**
   * @brief Get the set of [actions](@ref Action) in *done* state
   *
   * @return The set of [actions](@ref Action) in *done* state
   */
  virtual ActionListPtr getDoneActionSet() {return p_doneActionSet;}

  /**
   * @brief Get the set of modified [actions](@ref Action)
   *
   * @return The set of modified [actions](@ref Action)
   */
  virtual ActionLmmListPtr getModifiedSet() {return p_modifiedSet;}

  /**
   * @brief Get the maxmin system of the current Model
   *
   * @return The maxmin system of the current Model
   */
  lmm_system_t getMaxminSystem() {return p_maxminSystem;}

  /**
   * @brief Get the update mechanism of the current Model
   * @see e_UM_t
   *
   * @return [description]
   */
  e_UM_t getUpdateMechanism() {return p_updateMechanism;}

  /**
   * @brief Get Action heap
   * @details [TODO]
   *
   * @return The Action heap
   */
  xbt_heap_t getActionHeap() {return p_actionHeap;}

  /**
   * @brief share the resources
   * @details Share the resources between the actions
   *
   * @param now The current time of the simulation
   * @return The delta of time till the next action will finish
   */
  virtual double shareResources(double now);
  virtual double shareResourcesLazy(double now);
  virtual double shareResourcesFull(double now);
  double shareResourcesMaxMin(ActionListPtr running_actions,
                                      lmm_system_t sys,
                                      void (*solve) (lmm_system_t));

  /**
   * @brief Update state of actions
   * @details Update action to the current time
   *
   * @param now The current time of the simulation
   * @param delta The delta of time since the last update
   */
  virtual void updateActionsState(double now, double delta);
  virtual void updateActionsStateLazy(double now, double delta);
  virtual void updateActionsStateFull(double now, double delta);

protected:
  ActionLmmListPtr p_modifiedSet;
  lmm_system_t p_maxminSystem;
  e_UM_t p_updateMechanism;
  int m_selectiveUpdate;
  xbt_heap_t p_actionHeap;

private:
  const char *p_name;

  ActionListPtr p_readyActionSet; /**< Actions in state SURF_ACTION_READY */
  ActionListPtr p_runningActionSet; /**< Actions in state SURF_ACTION_RUNNING */
  ActionListPtr p_failedActionSet; /**< Actions in state SURF_ACTION_FAILED */
  ActionListPtr p_doneActionSet; /**< Actions in state SURF_ACTION_DONE */
};

/************
 * Resource *
 ************/

/** @ingroup SURF_interface
 * @brief Resource which have a metric handled by a maxmin system
 */
typedef struct {
  double scale;             /**< The scale of the metric */
  double peak;              /**< The peak of the metric */
  tmgr_trace_event_t event; /**< The associated trace event associated to the metric */
} s_surf_metric_t;

/** @ingroup SURF_interface
 * @brief SURF resource interface class
 * @details A resource represent an element of a component (e.g.: a link for the network)
 */
XBT_PUBLIC_CLASS Resource {
public:
  /**
   * @brief Resource constructor
   */
  Resource();

  /**
   * @brief Resource constructor
   *
   * @param model Model associated to this Resource
   * @param name The name of the Resource
   * @param props Dictionary of properties associated to this Resource
   */
  Resource(ModelPtr model, const char *name, xbt_dict_t props);

  /**
   * @brief Resource constructor
   *
   * @param model Model associated to this Resource
   * @param name The name of the Resource
   * @param props Dictionary of properties associated to this Resource
   * @param constraint The lmm constraint associated to this Resource if it is part of a LMM component
   */
  Resource(ModelPtr model, const char *name, xbt_dict_t props, lmm_constraint_t constraint);
  /**
   * @brief Resource constructor
   *
   * @param model Model associated to this Resource
   * @param name The name of the Resource
   * @param props Dictionary of properties associated to this Resource
   * @param stateInit the initial state of the Resource
   */
  Resource(ModelPtr model, const char *name, xbt_dict_t props, e_surf_resource_state_t stateInit);

  /**
   * @brief Resource destructor
   */
  virtual ~Resource();

  /**
   * @brief Get the Model of the current Resource
   *
   * @return The Model of the current Resource
   */
  ModelPtr getModel();

  /**
   * @brief Get the name of the current Resource
   *
   * @return The name of the current Resource
   */
  const char *getName();

  /**
   * @brief Get the properties of the current Resource
   *
   * @return The properties of the current Resource
   */
  virtual xbt_dict_t getProperties();

  /**
   * @brief Update the state of the current Resource
   * @details [TODO]
   *
   * @param event_type [TODO]
   * @param value [TODO]
   * @param date [TODO]
   */
  virtual void updateState(tmgr_trace_event_t event_type, double value, double date)=0;

  /**
   * @brief Check if the current Resource is used
   * @return true if the current Resource is used, false otherwise
   */
  virtual bool isUsed()=0;

  /**
   * @brief Check if the current Resource is active
   *
   * @return true if the current Resource is active, false otherwise
   */
  bool isOn();

  /**
   * @brief Turn on the current Resource
   */
  void turnOn();

  /**
   * @brief Turn off the current Resource
   */
  void turnOff();

  /**
   * @brief Get the [state](\ref e_surf_resource_state_t) of the current Resource
   *
   * @return The state of the currenrt Resource
   */
  virtual e_surf_resource_state_t getState();

  /**
   * @brief Set the [state](\ref e_surf_resource_state_t) of the current Resource
   *
   * @param state The new state of the current Resource
   */
  virtual void setState(e_surf_resource_state_t state);

private:
  const char *p_name;
  xbt_dict_t p_properties;
  ModelPtr p_model;
  bool m_running;
  e_surf_resource_state_t m_stateCurrent;

  /* LMM */
public:
  /**
   * @brief Get the lmm constraint associated to this Resource if it is part of a LMM component
   *
   * @return The lmm constraint associated to this Resource
   */
  lmm_constraint_t getConstraint();
private:
  lmm_constraint_t p_constraint;
};

/**********
 * Action *
 **********/
void surf_action_lmm_update_index_heap(void *action, int i);

/** @ingroup SURF_interface
 * @brief SURF action interface class
 * @details An action is an event generated by a resource (e.g.: a communication for the network)
 */
XBT_PUBLIC_CLASS Action : public actionHook, public actionLmmHook {
private:
  /**
   * @brief Common initializations for the constructors
   */
  void initialize(ModelPtr model, double cost, bool failed,
                  lmm_variable_t var = NULL);

public:
  /**
   * @brief Action constructor
   *
   * @param model The Model associated to this Action
   * @param cost The cost of the Action
   * @param failed If the action is impossible (e.g.: execute something on a switched off workstation)
   */
  Action(ModelPtr model, double cost, bool failed);

  /**
   * @brief Action constructor
   *
   * @param model The Model associated to this Action
   * @param cost The cost of the Action
   * @param failed If the action is impossible (e.g.: execute something on a switched off workstation)
   * @param var The lmm variable associated to this Action if it is part of a LMM component
   */
  Action(ModelPtr model, double cost, bool failed, lmm_variable_t var);

  /**
   * @brief Action destructor
   */
  virtual ~Action();

  /**
   * @brief Finish the action
   */
  void finish();

  /**
   * @brief Get the [state](\ref e_surf_action_state_t) of the current Action
   *
   * @return The state of the current Action
   */
  e_surf_action_state_t getState(); /**< get the state*/

  /**
   * @brief Set the [state](\ref e_surf_action_state_t) of the current Action
   *
   * @param state The new state of the current Action
   */
  virtual void setState(e_surf_action_state_t state);

  /**
   * @brief Get the bound of the current Action
   *
   * @return The bound of the current Action
   */
  double getBound();

  /**
   * @brief Set the bound of the current Action
   *
   * @param bound The new bound of the current Action
   */
  void setBound(double bound);

  /**
   * @brief Get the start time of the current action
   *
   * @return The start time of the current action
   */
  double getStartTime();

  /**
   * @brief Get the finish time of the current action
   *
   * @return The finish time of the current action
   */
  double getFinishTime();

  /**
   * @brief Get the data associated to the current action
   *
   * @return The data associated to the current action
   */
  void *getData() {return p_data;}

  /**
   * @brief Set the data associated to the current action
   *
   * @param data The new data associated to the current action
   */
  void setData(void* data);

  /**
   * @brief Get the maximum duration of the current action
   *
   * @return The maximum duration of the current action
   */
  double getMaxDuration() {return m_maxDuration;}

  /**
   * @brief Get the category associated to the current action
   *
   * @return The category associated to the current action
   */
  char *getCategory() {return p_category;}

  /**
   * @brief Get the cost of the current action
   *
   * @return The cost of the current action
   */
  double getCost() {return m_cost;}

  /**
   * @brief Set the cost of the current action
   *
   * @param cost The new cost of the current action
   */
  void setCost(double cost) {m_cost = cost;}

  /**
   * @brief Update the maximum duration of the current action
   *
   * @param delta [TODO]
   */
  void updateMaxDuration(double delta) {double_update(&m_maxDuration, delta,sg_surf_precision);}

  /**
   * @brief Update the remaining time of the current action
   *
   * @param delta [TODO]
   */
  void updateRemains(double delta) {double_update(&m_remains, delta, sg_maxmin_precision*sg_surf_precision);}

  /**
   * @brief Set the remaining time of the current action
   *
   * @param value The new remaining time of the current action
   */
  void setRemains(double value) {m_remains = value;}

  /**
   * @brief Set the finish time of the current action
   *
   * @param value The new Finush time of the current action
   */
  void setFinishTime(double value) {m_finish = value;}

  /**
   * @brief Add a reference to the current action
   */
  void ref();

  /**
   * @brief Remove a reference to the current action
   * @details If the Action has no more reference, we destroy it
   *
   * @return true if the action was destroyed and false if someone still has references on it
   */
  virtual int unref();

  /**
   * @brief Cancel the current Action if running
   */
  virtual void cancel();

  /**
   * @brief Recycle an Action
   */
  virtual void recycle(){};

  /**
   * @brief Suspend the current Action
   */
  virtual void suspend();

  /**
   * @brief Resume the current Action
   */
  virtual void resume();

  /**
   * @brief Check if the current action is running
   *
   * @return true if the current Action is suspended, false otherwise
   */
  virtual bool isSuspended();

  /**
   * @brief Set the maximum duration of the current Action
   *
   * @param duration The new maximum duration of the current Action
   */
  virtual void setMaxDuration(double duration);

  /**
   * @brief Set the priority of the current Action
   *
   * @param priority The new priority of the current Action
   */
  virtual void setPriority(double priority);

#ifdef HAVE_TRACING
  /**
   * @brief Set the category of the current Action
   *
   * @param category The new category of the current Action
   */
  void setCategory(const char *category);
#endif

  /**
   * @brief Get the remaining time of the current action after updating the resource
   *
   * @return The remaining time
   */
  virtual double getRemains();

  /**
   * @brief Get the remaining time of the current action without updating the resource
   *
   * @return The remaining time
   */
  double getRemainsNoUpdate();

  /**
   * @brief Get the priority of the current Action
   *
   * @return The priority of the current Action
   */
  double getPriority() {return m_priority;};

  /**
   * @brief Get the state set in which the action is
   *
   * @return The state set in which the action is
   */
  ActionListPtr getStateSet() {return p_stateSet;};

  s_xbt_swag_hookup_t p_stateHookup;

  ModelPtr getModel() {return p_model;}

protected:
  ActionListPtr p_stateSet;
  double m_priority; /**< priority (1.0 by default) */
  int    m_refcount;
  double m_remains; /**< How much of that cost remains to be done in the currently running task */
  double m_maxDuration; /*< max_duration (may fluctuate until the task is completed) */
  double m_finish; /**< finish time : this is modified during the run and fluctuates until the task is completed */

private:
  bool m_failed;
  double m_start; /**< start time  */
  char *p_category;               /**< tracing category for categorized resource utilization monitoring */

  #ifdef HAVE_LATENCY_BOUND_TRACKING
  int m_latencyLimited;               /**< Set to 1 if is limited by latency, 0 otherwise */
  #endif
  double    m_cost;
  ModelPtr p_model;
  void *p_data; /**< for your convenience */

  /* LMM */
public:
  virtual void updateRemainingLazy(double now);
  void heapInsert(xbt_heap_t heap, double key, enum heap_action_type hat);
  void heapRemove(xbt_heap_t heap);
  void heapUpdate(xbt_heap_t heap, double key, enum heap_action_type hat);
  void updateIndexHeap(int i);
  lmm_variable_t getVariable() {return p_variable;}
  double getLastUpdate() {return m_lastUpdate;}
  void refreshLastUpdate() {m_lastUpdate = surf_get_clock();}
  enum heap_action_type getHat() {return m_hat;}
  bool is_linked() {return actionLmmHook::is_linked();}
  void gapRemove();

protected:
  lmm_variable_t p_variable;
  double m_lastValue;
  double m_lastUpdate;
  int m_suspended;
  int m_indexHeap;
  enum heap_action_type m_hat;
};

#endif /* SURF_MODEL_H_ */
