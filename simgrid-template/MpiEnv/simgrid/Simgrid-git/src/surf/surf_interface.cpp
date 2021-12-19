/* Copyright (c) 2004-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "surf_private.h"
#include "surf_interface.hpp"
#include "network_interface.hpp"
#include "cpu_interface.hpp"
#include "workstation_interface.hpp"
#include "vm_workstation_interface.hpp"
#include "simix/smx_host_private.h"
#include "surf_routing.hpp"
#include "simgrid/sg_config.h"
#include "mc/mc.h"

XBT_LOG_NEW_CATEGORY(surf, "All SURF categories");
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(surf_kernel, surf,
                                "Logging specific to SURF (kernel)");

/*********
 * Utils *
 *********/

/* This function is a pimple that we ought to fix. But it won't be easy.
 *
 * The surf_solve() function does properly return the set of actions that
 * changed. Instead, each model change a global data, and then the caller of
 * surf_solve must pick into these sets of action_failed and action_done.
 *
 * This was not clean but ok as long as we didn't had to restart the processes
 * when the resource comes back up.
 * We worked by putting sentinel actions on every resources we are interested
 * in, so that surf informs us if/when the corresponding resource fails.
 *
 * But this does not work to get Simix informed of when a resource comes back
 * up, and this is where this pimple comes. We have a set of resources that are
 * currently down and for which simix needs to know when it comes back up.
 * And the current function is called *at every simulation step* to sweep over
 * that set, searching for a resource that was turned back up in the meanwhile.
 * This is UGLY and slow.
 *
 * The proper solution would be to not rely on globals for the action_failed and
 * action_done swags. They must be passed as parameter by the caller (the
 * handling of these actions in simix may let you think that these two sets can
 * be merged, but their handling in SimDag induce the contrary unless this
 * simdag code can check by itself whether the action is done of failed -- seems
 * very doable, but yet more cleanup to do).
 *
 * Once surf_solve() is passed the set of actions that changed, you want to add
 * a new set of resources back up as parameter to this function. You also want
 * to add a boolean field "restart_watched" to each resource, and make sure that
 * whenever a resource with this field enabled comes back up, it's added to that
 * set so that Simix sees it and react accordingly. This would kill that need
 * for surf to call simix.
 *
 */

/*static void remove_watched_host(void *key)
{
  xbt_dict_remove(watched_hosts_lib, *(char**)key);
}*/

/*void surf_watched_hosts(void)
{
  char *key;
  void *host;
  xbt_dict_cursor_t cursor;
  xbt_dynar_t hosts = xbt_dynar_new(sizeof(char*), NULL);

  XBT_DEBUG("Check for host SURF_RESOURCE_ON on watched_hosts_lib");
  xbt_dict_foreach(watched_hosts_lib, cursor, key, host)
  {
    if(SIMIX_host_get_state((smx_host_t)host) == SURF_RESOURCE_ON){
      XBT_INFO("Restart processes on host: %s", SIMIX_host_get_name((smx_host_t)host));
      SIMIX_host_autorestart((smx_host_t)host);
      xbt_dynar_push_as(hosts, char*, key);
    }
    else
      XBT_DEBUG("See SURF_RESOURCE_OFF on host: %s",key);
  }
  xbt_dynar_map(hosts, remove_watched_host);
  xbt_dynar_free(&hosts);
}*/

/* model_list_invoke contains only surf_workstation and surf_vm_workstation.
 * The callback functions of cpu_model and network_model will be called from
 * those of these workstation models. */
xbt_dynar_t model_list = NULL; /* for destroying all models correctly */
xbt_dynar_t model_list_invoke = NULL;  /* for invoking callbacks */

tmgr_history_t history = NULL;
lmm_system_t maxmin_system = NULL;
xbt_dynar_t surf_path = NULL;
xbt_dynar_t host_that_restart = NULL;
xbt_dict_t watched_hosts_lib;

surf_callback(void, void) surfExitCallbacks;

s_surf_model_description_t surf_plugin_description[] = {
    {"Energy",
     "Cpu energy consumption.",
     sg_energy_plugin_init},
     {NULL, NULL,  NULL}      /* this array must be NULL terminated */
};

/* Don't forget to update the option description in smx_config when you change
   this */
s_surf_model_description_t surf_network_model_description[] = {
  {"LV08",
   "Realistic network analytic model (slow-start modeled by multiplying latency by 10.4, bandwidth by .92; bottleneck sharing uses a payload of S=8775 for evaluating RTT). ",
   surf_network_model_init_LegrandVelho},
  {"Constant",
   "Simplistic network model where all communication take a constant time (one second). This model provides the lowest realism, but is (marginally) faster.",
   surf_network_model_init_Constant},
  {"SMPI",
   "Realistic network model specifically tailored for HPC settings (accurate modeling of slow start with correction factors on three intervals: < 1KiB, < 64 KiB, >= 64 KiB)",
   surf_network_model_init_SMPI},
  {"IB",
   "Realistic network model specifically tailored for HPC settings, with Infiniband contention model",
   surf_network_model_init_IB},
  {"CM02",
   "Legacy network analytic model (Very similar to LV08, but without corrective factors. The timings of small messages are thus poorly modeled).",
   surf_network_model_init_CM02},
#ifdef HAVE_GTNETS
  {"GTNets",
   "Network pseudo-model using the GTNets simulator instead of an analytic model",
   surf_network_model_init_GTNETS},
#endif
#ifdef HAVE_NS3
  {"NS3",
   "Network pseudo-model using the NS3 tcp model instead of an analytic model",
  surf_network_model_init_NS3},
#endif
  {"Reno",
   "Model from Steven H. Low using lagrange_solve instead of lmm_solve (experts only; check the code for more info).",
   surf_network_model_init_Reno},
  {"Reno2",
   "Model from Steven H. Low using lagrange_solve instead of lmm_solve (experts only; check the code for more info).",
   surf_network_model_init_Reno2},
  {"Vegas",
   "Model from Steven H. Low using lagrange_solve instead of lmm_solve (experts only; check the code for more info).",
   surf_network_model_init_Vegas},
  {NULL, NULL, NULL}      /* this array must be NULL terminated */
};

s_surf_model_description_t surf_cpu_model_description[] = {
  {"Cas01",
   "Simplistic CPU model (time=size/power).",
   surf_cpu_model_init_Cas01},
  {NULL, NULL,  NULL}      /* this array must be NULL terminated */
};

s_surf_model_description_t surf_workstation_model_description[] = {
  {"default",
   "Default workstation model. Currently, CPU:Cas01 and network:LV08 (with cross traffic enabled)",
   surf_workstation_model_init_current_default},
  {"compound",
   "Workstation model that is automatically chosen if you change the network and CPU models",
   surf_workstation_model_init_compound},
  {"ptask_L07", "Workstation model somehow similar to Cas01+CM02 but allowing parallel tasks",
   surf_workstation_model_init_ptask_L07},
  {NULL, NULL, NULL}      /* this array must be NULL terminated */
};

s_surf_model_description_t surf_vm_workstation_model_description[] = {
  {"default",
   "Default vm workstation model.",
   surf_vm_workstation_model_init_HL13},
  {NULL, NULL, NULL}      /* this array must be NULL terminated */
};

s_surf_model_description_t surf_optimization_mode_description[] = {
  {"Lazy",
   "Lazy action management (partial invalidation in lmm + heap in action remaining).",
   NULL},
  {"TI",
   "Trace integration. Highly optimized mode when using availability traces (only available for the Cas01 CPU model for now).",
    NULL},
  {"Full",
   "Full update of remaining and variables. Slow but may be useful when debugging.",
   NULL},
  {NULL, NULL, NULL}      /* this array must be NULL terminated */
};

s_surf_model_description_t surf_storage_model_description[] = {
  {"default",
   "Simplistic storage model.",
   surf_storage_model_init_default},
  {NULL, NULL,  NULL}      /* this array must be NULL terminated */
};

#ifdef CONTEXT_THREADS
static xbt_parmap_t surf_parmap = NULL; /* parallel map on models */
#endif

double NOW = 0;
double *surf_mins = NULL; /* return value of share_resources for each model */
int surf_min_index;       /* current index in surf_mins */
double surf_min;               /* duration determined by surf_solve */

double surf_get_clock(void)
{
  return NOW;
}

#ifdef _XBT_WIN32
# define FILE_DELIM "\\"
#else
# define FILE_DELIM "/"         /* FIXME: move to better location */
#endif

FILE *surf_fopen(const char *name, const char *mode)
{
  unsigned int cpt;
  char *path_elm = NULL;
  char *buff;
  FILE *file = NULL;

  xbt_assert(name);

  if (__surf_is_absolute_file_path(name))       /* don't mess with absolute file names */
    return fopen(name, mode);

  /* search relative files in the path */
  xbt_dynar_foreach(surf_path, cpt, path_elm) {
    buff = bprintf("%s" FILE_DELIM "%s", path_elm, name);
    file = fopen(buff, mode);
    free(buff);

    if (file)
      return file;
  }
  return NULL;
}


#ifndef MAX_DRIVE
#define MAX_DRIVE 26
#endif

#ifdef _XBT_WIN32
#include <windows.h>
static const char *disk_drives_letter_table[MAX_DRIVE] = {
  "A:\\",
  "B:\\",
  "C:\\",
  "D:\\",
  "E:\\",
  "F:\\",
  "G:\\",
  "H:\\",
  "I:\\",
  "J:\\",
  "K:\\",
  "L:\\",
  "M:\\",
  "N:\\",
  "O:\\",
  "P:\\",
  "Q:\\",
  "R:\\",
  "S:\\",
  "T:\\",
  "U:\\",
  "V:\\",
  "W:\\",
  "X:\\",
  "Y:\\",
  "Z:\\"
};
#endif

/*
 * Returns the initial path. On Windows the initial path is
 * the current directory for the current process in the other
 * case the function returns "./" that represents the current
 * directory on Unix/Linux platforms.
 */

const char *__surf_get_initial_path(void)
{

#ifdef _XBT_WIN32
  unsigned i;
  char current_directory[MAX_PATH + 1] = { 0 };
  unsigned int len = GetCurrentDirectory(MAX_PATH + 1, current_directory);
  char root[4] = { 0 };

  if (!len)
    return NULL;

  strncpy(root, current_directory, 3);

  for (i = 0; i < MAX_DRIVE; i++) {
    if (toupper(root[0]) == disk_drives_letter_table[i][0])
      return disk_drives_letter_table[i];
  }

  return NULL;
#else
  return "./";
#endif
}

/* The __surf_is_absolute_file_path() returns 1 if
 * file_path is a absolute file path, in the other
 * case the function returns 0.
 */
int __surf_is_absolute_file_path(const char *file_path)
{
#ifdef _XBT_WIN32
  WIN32_FIND_DATA wfd = { 0 };
  HANDLE hFile = FindFirstFile(file_path, &wfd);

  if (INVALID_HANDLE_VALUE == hFile)
    return 0;

  FindClose(hFile);
  return 1;
#else
  return (file_path[0] == '/');
#endif
}

/** Displays the long description of all registered models, and quit */
void model_help(const char *category, s_surf_model_description_t * table)
{
  int i;
  printf("Long description of the %s models accepted by this simulator:\n",
         category);
  for (i = 0; table[i].name; i++)
    printf("  %s: %s\n", table[i].name, table[i].description);
}

int find_model_description(s_surf_model_description_t * table,
                           const char *name)
{
  int i;
  char *name_list = NULL;

  for (i = 0; table[i].name; i++)
    if (!strcmp(name, table[i].name)) {
      return i;
    }
  if (!table[0].name)
    xbt_die("No model is valid! This is a bug.");
  name_list = xbt_strdup(table[0].name);
  for (i = 1; table[i].name; i++) {
    name_list = (char *) xbt_realloc(name_list, strlen(name_list) + strlen(table[i].name) + 3);
    strcat(name_list, ", ");
    strcat(name_list, table[i].name);
  }
  xbt_die("Model '%s' is invalid! Valid models are: %s.", name, name_list);
  return -1;
}

static XBT_INLINE void routing_asr_host_free(void *p)
{
  delete ((RoutingEdgePtr) p);
}

static XBT_INLINE void routing_asr_prop_free(void *p)
{
  xbt_dict_t elm = (xbt_dict_t) p;
  xbt_dict_free(&elm);
}

static XBT_INLINE void surf_cpu_free(void *r)
{
  delete static_cast<CpuPtr>(r);
}

static XBT_INLINE void surf_link_free(void *r)
{
  delete static_cast<NetworkLinkPtr>(r);
}

static XBT_INLINE void surf_workstation_free(void *r)
{
  delete static_cast<WorkstationPtr>(r);
}

static XBT_INLINE void surf_storage_free(void *r)
{
  delete static_cast<StoragePtr>(r);
}


void sg_version(int *ver_major,int *ver_minor,int *ver_patch) {
  *ver_major = SIMGRID_VERSION_MAJOR;
  *ver_minor = SIMGRID_VERSION_MINOR;
  *ver_patch = SIMGRID_VERSION_PATCH;
}

void surf_init(int *argc, char **argv)
{
  XBT_DEBUG("Create all Libs");
  host_lib = xbt_lib_new();
  link_lib = xbt_lib_new();
  as_router_lib = xbt_lib_new();
  storage_lib = xbt_lib_new();
  storage_type_lib = xbt_lib_new();
  file_lib = xbt_lib_new();
  watched_hosts_lib = xbt_dict_new_homogeneous(NULL);

  XBT_DEBUG("Add routing levels");
  ROUTING_HOST_LEVEL = xbt_lib_add_level(host_lib,routing_asr_host_free);
  ROUTING_ASR_LEVEL  = xbt_lib_add_level(as_router_lib,routing_asr_host_free);
  ROUTING_PROP_ASR_LEVEL = xbt_lib_add_level(as_router_lib,routing_asr_prop_free);

  XBT_DEBUG("Add SURF levels");
  SURF_CPU_LEVEL = xbt_lib_add_level(host_lib,surf_cpu_free);
  SURF_WKS_LEVEL = xbt_lib_add_level(host_lib,surf_workstation_free);
  SURF_LINK_LEVEL = xbt_lib_add_level(link_lib,surf_link_free);
  SURF_STORAGE_LEVEL = xbt_lib_add_level(storage_lib,surf_storage_free);

  xbt_init(argc, argv);
  if (!model_list)
    model_list = xbt_dynar_new(sizeof(ModelPtr), NULL);
  if (!model_list_invoke)
    model_list_invoke = xbt_dynar_new(sizeof(ModelPtr), NULL);
  if (!history)
    history = tmgr_history_new();

#ifdef HAVE_TRACING
  TRACE_add_start_function(TRACE_surf_alloc);
  TRACE_add_end_function(TRACE_surf_release);
#endif

  sg_config_init(argc, argv);

  if (MC_is_active())
    MC_memory_init();
}

void surf_exit(void)
{
  unsigned int iter;
  ModelPtr model = NULL;

#ifdef HAVE_TRACING
  TRACE_end();                  /* Just in case it was not called by the upper
                                 * layer (or there is no upper layer) */
#endif

  sg_config_finalize();

  xbt_dynar_free(&host_that_restart);
  xbt_dynar_free(&surf_path);

  xbt_lib_free(&host_lib);
  xbt_lib_free(&link_lib);
  xbt_lib_free(&as_router_lib);
  xbt_lib_free(&storage_lib);
  xbt_lib_free(&storage_type_lib);
  xbt_lib_free(&file_lib);
  xbt_dict_free(&watched_hosts_lib);

  xbt_dynar_foreach(model_list, iter, model)
    delete model;
  xbt_dynar_free(&model_list);
  xbt_dynar_free(&model_list_invoke);
  routing_exit();

  surf_callback_emit(surfExitCallbacks);

  if (maxmin_system) {
    lmm_system_free(maxmin_system);
    maxmin_system = NULL;
  }
  if (history) {
    tmgr_history_free(history);
    history = NULL;
  }

#ifdef CONTEXT_THREADS
  xbt_parmap_destroy(surf_parmap);
#endif

  xbt_free(surf_mins);
  surf_mins = NULL;

  tmgr_finalize();
  surf_parse_lex_destroy();
  surf_parse_free_callbacks();

  NOW = 0;                      /* Just in case the user plans to restart the simulation afterward */
}

/*********
 * Model *
 *********/

Model::Model(const char *name)
  : p_maxminSystem(NULL)
  , p_name(name)
{
  p_readyActionSet = new ActionList();
  p_runningActionSet = new ActionList();
  p_failedActionSet = new ActionList();
  p_doneActionSet = new ActionList();

  p_modifiedSet = NULL;
  p_actionHeap = NULL;
  p_updateMechanism = UM_UNDEFINED;
  m_selectiveUpdate = 0;
}

Model::~Model(){
  delete p_readyActionSet;
  delete p_runningActionSet;
  delete p_failedActionSet;
  delete p_doneActionSet;
}

double Model::shareResources(double now)
{
  //FIXME: set the good function once and for all
  if (p_updateMechanism == UM_LAZY)
    return shareResourcesLazy(now);
  else if (p_updateMechanism == UM_FULL)
    return shareResourcesFull(now);
  else
    xbt_die("Invalid cpu update mechanism!");
}

double Model::shareResourcesLazy(double now)
{
  ActionPtr action = NULL;
  double min = -1;
  double share;

  XBT_DEBUG
      ("Before share resources, the size of modified actions set is %zd",
       p_modifiedSet->size());

  lmm_solve(p_maxminSystem);

  XBT_DEBUG
      ("After share resources, The size of modified actions set is %zd",
       p_modifiedSet->size());

  while(!p_modifiedSet->empty()) {
    action = &(p_modifiedSet->front());
    p_modifiedSet->pop_front();
    int max_dur_flag = 0;

    if (action->getStateSet() != p_runningActionSet)
      continue;

    /* bogus priority, skip it */
    if (action->getPriority() <= 0 || action->getHat()==LATENCY)
      continue;

    action->updateRemainingLazy(now);

    min = -1;
    share = lmm_variable_getvalue(action->getVariable());

    if (share > 0) {
      double time_to_completion;
      if (action->getRemains() > 0) {
        time_to_completion = action->getRemainsNoUpdate() / share;
      } else {
        time_to_completion = 0.0;
      }
      min = now + time_to_completion; // when the task will complete if nothing changes
    }

    if ((action->getMaxDuration() != NO_MAX_DURATION)
        && (min == -1
            || action->getStartTime() +
            action->getMaxDuration() < min)) {
      min = action->getStartTime() +
          action->getMaxDuration();  // when the task will complete anyway because of the deadline if any
      max_dur_flag = 1;
    }


    XBT_DEBUG("Action(%p) corresponds to variable %d", action, action->getVariable()->id_int);

    XBT_DEBUG("Action(%p) Start %f. May finish at %f (got a share of %f). Max_duration %f", action,
        action->getStartTime(), min, share,
        action->getMaxDuration());

    if (min != -1) {
      action->heapUpdate(p_actionHeap, min, max_dur_flag ? MAX_DURATION : NORMAL);
      XBT_DEBUG("Insert at heap action(%p) min %f now %f", action, min,
                now);
    } else DIE_IMPOSSIBLE;
  }

  //hereafter must have already the min value for this resource model
  if (xbt_heap_size(p_actionHeap) > 0)
    min = xbt_heap_maxkey(p_actionHeap) - now;
  else
    min = -1;

  XBT_DEBUG("The minimum with the HEAP %f", min);

  return min;
}

double Model::shareResourcesFull(double /*now*/) {
  THROW_UNIMPLEMENTED;
}

double Model::shareResourcesMaxMin(ActionListPtr running_actions,
                          lmm_system_t sys,
                          void (*solve) (lmm_system_t))
{
  ActionPtr action = NULL;
  double min = -1;
  double value = -1;

  solve(sys);

  ActionList::iterator it(running_actions->begin()), itend(running_actions->end());
  for(; it != itend ; ++it) {
    action = &*it;
    value = lmm_variable_getvalue(action->getVariable());
    if ((value > 0) || (action->getMaxDuration() >= 0))
      break;
  }

  if (!action)
    return -1.0;

  if (value > 0) {
    if (action->getRemains() > 0)
      min = action->getRemainsNoUpdate() / value;
    else
      min = 0.0;
    if ((action->getMaxDuration() >= 0) && (action->getMaxDuration() < min))
      min = action->getMaxDuration();
  } else
    min = action->getMaxDuration();


  for (++it; it != itend; ++it) {
	action = &*it;
    value = lmm_variable_getvalue(action->getVariable());
    if (value > 0) {
      if (action->getRemains() > 0)
        value = action->getRemainsNoUpdate() / value;
      else
        value = 0.0;
      if (value < min) {
        min = value;
        XBT_DEBUG("Updating min (value) with %p: %f", action, min);
      }
    }
    if ((action->getMaxDuration() >= 0) && (action->getMaxDuration() < min)) {
      min = action->getMaxDuration();
      XBT_DEBUG("Updating min (duration) with %p: %f", action, min);
    }
  }
  XBT_DEBUG("min value : %f", min);

  return min;
}

void Model::updateActionsState(double now, double delta)
{
  if (p_updateMechanism == UM_FULL)
	updateActionsStateFull(now, delta);
  else if (p_updateMechanism == UM_LAZY)
	updateActionsStateLazy(now, delta);
  else
	xbt_die("Invalid cpu update mechanism!");
}

void Model::updateActionsStateLazy(double /*now*/, double /*delta*/)
{
 THROW_UNIMPLEMENTED;
}

void Model::updateActionsStateFull(double /*now*/, double /*delta*/)
{
  THROW_UNIMPLEMENTED;
}

/************
 * Resource *
 ************/

Resource::Resource()
: p_name(NULL), p_properties(NULL), p_model(NULL)
{}

Resource::Resource(ModelPtr model, const char *name, xbt_dict_t props)
  : p_name(xbt_strdup(name)), p_properties(props), p_model(model)
  , m_running(true), m_stateCurrent(SURF_RESOURCE_ON)
{}

Resource::Resource(ModelPtr model, const char *name, xbt_dict_t props, lmm_constraint_t constraint)
  : p_name(xbt_strdup(name)), p_properties(props), p_model(model)
  , m_running(true), m_stateCurrent(SURF_RESOURCE_ON), p_constraint(constraint)
{}

Resource::Resource(ModelPtr model, const char *name, xbt_dict_t props, e_surf_resource_state_t stateInit)
  : p_name(xbt_strdup(name)), p_properties(props), p_model(model)
  , m_running(true), m_stateCurrent(stateInit)
{}

Resource::~Resource() {
  xbt_free((void*)p_name);
  xbt_dict_free(&p_properties);
}

e_surf_resource_state_t Resource::getState()
{
  return m_stateCurrent;
}

void Resource::setState(e_surf_resource_state_t state)
{
  m_stateCurrent = state;
}

bool Resource::isOn()
{
  return m_running;
}

void Resource::turnOn()
{
  if (!m_running) {
    m_running = true;
  }
}

void Resource::turnOff()
{
  if (m_running) {
    m_running = false;
  }
}

ModelPtr Resource::getModel() {
  return p_model;
}

const char *Resource::getName() {
  return p_name;
}

xbt_dict_t Resource::getProperties() {
  if (p_properties==NULL)
    p_properties = xbt_dict_new();
  return p_properties;
}

lmm_constraint_t Resource::getConstraint() {
  return p_constraint;
}

/**********
 * Action *
 **********/

const char *surf_action_state_names[6] = {
  "SURF_ACTION_READY",
  "SURF_ACTION_RUNNING",
  "SURF_ACTION_FAILED",
  "SURF_ACTION_DONE",
  "SURF_ACTION_TO_FREE",
  "SURF_ACTION_NOT_IN_THE_SYSTEM"
};

void Action::initialize(ModelPtr model, double cost, bool failed,
                        lmm_variable_t var)
{
  m_priority = 1.0;
  m_refcount = 1;
  m_remains = cost;
  m_maxDuration = NO_MAX_DURATION;
  m_finish = -1.0;
  m_failed = failed;
  m_start = surf_get_clock();
  m_cost = cost;
  p_model = model;
  p_data = NULL;
  p_variable = var;
  m_lastValue = 0;
  m_lastUpdate = 0;
  m_suspended = false;
  m_hat = NOTSET;
}

Action::Action(ModelPtr model, double cost, bool failed)
{
  initialize(model, cost, failed);
  #ifdef HAVE_TRACING
    p_category = NULL;
  #endif
  p_stateHookup.prev = 0;
  p_stateHookup.next = 0;
  if (failed)
    p_stateSet = getModel()->getFailedActionSet();
  else
    p_stateSet = getModel()->getRunningActionSet();

  p_stateSet->push_back(*this);
}

Action::Action(ModelPtr model, double cost, bool failed, lmm_variable_t var)
{
  initialize(model, cost, failed, var);
  #ifdef HAVE_TRACING
    p_category = NULL;
  #endif
  p_stateHookup.prev = 0;
  p_stateHookup.next = 0;
  if (failed)
    p_stateSet = getModel()->getFailedActionSet();
  else
    p_stateSet = getModel()->getRunningActionSet();

  p_stateSet->push_back(*this);
}

Action::~Action() {
#ifdef HAVE_TRACING
  xbt_free(p_category);
#endif
}

void Action::finish() {
    m_finish = surf_get_clock();
}

e_surf_action_state_t Action::getState()
{
  if (p_stateSet ==  getModel()->getReadyActionSet())
    return SURF_ACTION_READY;
  if (p_stateSet ==  getModel()->getRunningActionSet())
    return SURF_ACTION_RUNNING;
  if (p_stateSet ==  getModel()->getFailedActionSet())
    return SURF_ACTION_FAILED;
  if (p_stateSet ==  getModel()->getDoneActionSet())
    return SURF_ACTION_DONE;
  return SURF_ACTION_NOT_IN_THE_SYSTEM;
}

void Action::setState(e_surf_action_state_t state)
{
  //surf_action_state_t action_state = &(action->model_type->states);
  XBT_IN("(%p,%s)", this, surf_action_state_names[state]);
  p_stateSet->erase(p_stateSet->iterator_to(*this));
  if (state == SURF_ACTION_READY)
    p_stateSet = getModel()->getReadyActionSet();
  else if (state == SURF_ACTION_RUNNING)
    p_stateSet = getModel()->getRunningActionSet();
  else if (state == SURF_ACTION_FAILED)
    p_stateSet = getModel()->getFailedActionSet();
  else if (state == SURF_ACTION_DONE)
    p_stateSet = getModel()->getDoneActionSet();
  else
    p_stateSet = NULL;

  if (p_stateSet)
    p_stateSet->push_back(*this);
  XBT_OUT();
}

double Action::getBound()
{
  return (p_variable) ? lmm_variable_getbound(p_variable) : 0;
}

void Action::setBound(double bound)
{
  XBT_IN("(%p,%g)", this, bound);
  if (p_variable)
    lmm_update_variable_bound(getModel()->getMaxminSystem(), p_variable, bound);

  if (getModel()->getUpdateMechanism() == UM_LAZY && getLastUpdate()!=surf_get_clock())
    heapRemove(getModel()->getActionHeap());
  XBT_OUT();
}

double Action::getStartTime()
{
  return m_start;
}

double Action::getFinishTime()
{
  /* keep the function behavior, some models (cpu_ti) change the finish time before the action end */
  return m_remains == 0 ? m_finish : -1;
}

void Action::setData(void* data)
{
  p_data = data;
}

#ifdef HAVE_TRACING
void Action::setCategory(const char *category)
{
  XBT_IN("(%p,%s)", this, category);
  p_category = xbt_strdup(category);
  XBT_OUT();
}
#endif

void Action::ref(){
  m_refcount++;
}

void Action::setMaxDuration(double duration)
{
  XBT_IN("(%p,%g)", this, duration);
  m_maxDuration = duration;
  if (getModel()->getUpdateMechanism() == UM_LAZY)      // remove action from the heap
    heapRemove(getModel()->getActionHeap());
  XBT_OUT();
}

void Action::gapRemove() {}

void Action::setPriority(double priority)
{
  XBT_IN("(%p,%g)", this, priority);
  m_priority = priority;
  lmm_update_variable_weight(getModel()->getMaxminSystem(), getVariable(), priority);

  if (getModel()->getUpdateMechanism() == UM_LAZY)
	heapRemove(getModel()->getActionHeap());
  XBT_OUT();
}

void Action::cancel(){
  setState(SURF_ACTION_FAILED);
  if (getModel()->getUpdateMechanism() == UM_LAZY) {
    if (actionLmmHook::is_linked())
      getModel()->getModifiedSet()->erase(getModel()->getModifiedSet()->iterator_to(*this));
    heapRemove(getModel()->getActionHeap());
  }
}

int Action::unref(){
  m_refcount--;
  if (!m_refcount) {
    if (actionHook::is_linked())
      p_stateSet->erase(p_stateSet->iterator_to(*this));
    if (getVariable())
      lmm_variable_free(getModel()->getMaxminSystem(), getVariable());
    if (getModel()->getUpdateMechanism() == UM_LAZY) {
      /* remove from heap */
      heapRemove(getModel()->getActionHeap());
      if (actionLmmHook::is_linked())
        getModel()->getModifiedSet()->erase(getModel()->getModifiedSet()->iterator_to(*this));
    }
    delete this;
    return 1;
  }
  return 0;
}

void Action::suspend()
{
  XBT_IN("(%p)", this);
  if (m_suspended != 2) {
    lmm_update_variable_weight(getModel()->getMaxminSystem(), getVariable(), 0.0);
    m_suspended = 1;
    if (getModel()->getUpdateMechanism() == UM_LAZY)
      heapRemove(getModel()->getActionHeap());
  }
  XBT_OUT();
}

void Action::resume()
{
  XBT_IN("(%p)", this);
  if (m_suspended != 2) {
    lmm_update_variable_weight(getModel()->getMaxminSystem(), getVariable(), m_priority);
    m_suspended = 0;
    if (getModel()->getUpdateMechanism() == UM_LAZY)
      heapRemove(getModel()->getActionHeap());
  }
  XBT_OUT();
}

bool Action::isSuspended()
{
  return m_suspended == 1;
}
/* insert action on heap using a given key and a hat (heap_action_type)
 * a hat can be of three types for communications:
 *
 * NORMAL = this is a normal heap entry stating the date to finish transmitting
 * LATENCY = this is a heap entry to warn us when the latency is payed
 * MAX_DURATION =this is a heap entry to warn us when the max_duration limit is reached
 */
void Action::heapInsert(xbt_heap_t heap, double key, enum heap_action_type hat)
{
  m_hat = hat;
  xbt_heap_push(heap, this, key);
}

void Action::heapRemove(xbt_heap_t heap)
{
  m_hat = NOTSET;
  if (m_indexHeap >= 0) {
    xbt_heap_remove(heap, m_indexHeap);
  }
}

void Action::heapUpdate(xbt_heap_t heap, double key, enum heap_action_type hat)
{
  m_hat = hat;
  if (m_indexHeap >= 0) {
    xbt_heap_update(heap, m_indexHeap, key);
  }else{
    xbt_heap_push(heap, this, key);
  }
}

/* added to manage the communication action's heap */
void surf_action_lmm_update_index_heap(void *action, int i) {
  ((ActionPtr)action)->updateIndexHeap(i);
}

void Action::updateIndexHeap(int i) {
  m_indexHeap = i;
}

double Action::getRemains()
{
  XBT_IN("(%p)", this);
  /* update remains before return it */
  if (getModel()->getUpdateMechanism() == UM_LAZY)      /* update remains before return it */
    updateRemainingLazy(surf_get_clock());
  XBT_OUT();
  return m_remains;
}

double Action::getRemainsNoUpdate()
{
  return m_remains;
}

//FIXME split code in the right places
void Action::updateRemainingLazy(double now)
{
  double delta = 0.0;

  if(getModel() == surf_network_model)
  {
    if (m_suspended != 0)
      return;
  }
  else
  {
    xbt_assert(p_stateSet == getModel()->getRunningActionSet(),
        "You're updating an action that is not running.");

      /* bogus priority, skip it */
    xbt_assert(m_priority > 0,
        "You're updating an action that seems suspended.");
  }

  delta = now - m_lastUpdate;

  if (m_remains > 0) {
    XBT_DEBUG("Updating action(%p): remains was %f, last_update was: %f", this, m_remains, m_lastUpdate);
    double_update(&m_remains, m_lastValue * delta, sg_surf_precision*sg_maxmin_precision);

#ifdef HAVE_TRACING
    if (getModel() == surf_cpu_model_pm && TRACE_is_enabled()) {
      ResourcePtr cpu = static_cast<ResourcePtr>(lmm_constraint_id(lmm_get_cnst_from_var(getModel()->getMaxminSystem(), getVariable(), 0)));
      TRACE_surf_host_set_utilization(cpu->getName(), getCategory(), m_lastValue, m_lastUpdate, now - m_lastUpdate);
    }
#endif
    XBT_DEBUG("Updating action(%p): remains is now %f", this, m_remains);
  }

  if(getModel() == surf_network_model)
  {
    if (m_maxDuration != NO_MAX_DURATION)
      double_update(&m_maxDuration, delta, sg_surf_precision);

    //FIXME: duplicated code
    if ((m_remains <= 0) &&
        (lmm_get_variable_weight(getVariable()) > 0)) {
      finish();
      setState(SURF_ACTION_DONE);
      heapRemove(getModel()->getActionHeap());
    } else if (((m_maxDuration != NO_MAX_DURATION)
        && (m_maxDuration <= 0))) {
      finish();
      setState(SURF_ACTION_DONE);
      heapRemove(getModel()->getActionHeap());
    }
  }

  m_lastUpdate = now;
  m_lastValue = lmm_variable_getvalue(getVariable());
}

