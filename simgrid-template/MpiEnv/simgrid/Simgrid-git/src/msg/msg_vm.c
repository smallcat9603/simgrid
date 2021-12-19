/* Copyright (c) 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/* TODO:
 * 1. add the support of trace
 * 2. use parallel tasks to simulate CPU overhead and remove the very
 *    experimental code generating micro computation tasks
 */



#include "msg_private.h"
#include "xbt/sysdep.h"
#include "xbt/log.h"
#include "simgrid/platf.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(msg_vm, msg,
                                "Cloud-oriented parts of the MSG API");


/* **** ******** GENERAL ********* **** */

/** \ingroup m_vm_management
 * \brief Returns the value of a given vm property
 *
 * \param vm a vm
 * \param name a property name
 * \return value of a property (or NULL if property not set)
 */

const char *MSG_vm_get_property_value(msg_vm_t vm, const char *name)
{
  return MSG_host_get_property_value(vm, name);
}

/** \ingroup m_vm_management
 * \brief Returns a xbt_dict_t consisting of the list of properties assigned to this host
 *
 * \param vm a vm
 * \return a dict containing the properties
 */
xbt_dict_t MSG_vm_get_properties(msg_vm_t vm)
{
  xbt_assert((vm != NULL), "Invalid parameters (vm is NULL)");

  return (simcall_host_get_properties(vm));
}

/** \ingroup m_host_management
 * \brief Change the value of a given host property
 *
 * \param vm a vm
 * \param name a property name
 * \param value what to change the property to
 * \param free_ctn the freeing function to use to kill the value on need
 */
void MSG_vm_set_property_value(msg_vm_t vm, const char *name, void *value, void_f_pvoid_t free_ctn)
{
  xbt_dict_set(MSG_host_get_properties(vm), name, value, free_ctn);
}

/** \ingroup msg_vm_management
 * \brief Finds a msg_vm_t using its name.
 *
 * This is a name directory service
 * \param name the name of a vm.
 * \return the corresponding vm
 *
 * Please note that a VM is a specific host. Hence, you should give a different name
 * for each VM/PM.
 */

msg_vm_t MSG_vm_get_by_name(const char *name)
{
	return MSG_get_host_by_name(name);
}

/** \ingroup m_vm_management
 *
 * \brief Return the name of the #msg_host_t.
 *
 * This functions checks whether \a host is a valid pointer or not and return
   its name.
 */
const char *MSG_vm_get_name(msg_vm_t vm)
{
  return MSG_host_get_name(vm);
}


/* **** Check state of a VM **** */
static inline int __MSG_vm_is_state(msg_vm_t vm, e_surf_vm_state_t state)
{
  return simcall_vm_get_state(vm) == state;
}

/** @brief Returns whether the given VM has just created, not running.
 *  @ingroup msg_VMs
 */
int MSG_vm_is_created(msg_vm_t vm)
{
  return __MSG_vm_is_state(vm, SURF_VM_STATE_CREATED);
}

/** @brief Returns whether the given VM is currently running
 *  @ingroup msg_VMs
 */
int MSG_vm_is_running(msg_vm_t vm)
{
  return __MSG_vm_is_state(vm, SURF_VM_STATE_RUNNING);
}

/** @brief Returns whether the given VM is currently migrating
 *  @ingroup msg_VMs
 */
int MSG_vm_is_migrating(msg_vm_t vm)
{
  msg_host_priv_t priv = msg_host_resource_priv(vm);
  return priv->is_migrating;
}

/** @brief Returns whether the given VM is currently suspended, not running.
 *  @ingroup msg_VMs
 */
int MSG_vm_is_suspended(msg_vm_t vm)
{
  return __MSG_vm_is_state(vm, SURF_VM_STATE_SUSPENDED);
}

/** @brief Returns whether the given VM is being saved (FIXME: live saving or not?).
 *  @ingroup msg_VMs
 */
int MSG_vm_is_saving(msg_vm_t vm)
{
  return __MSG_vm_is_state(vm, SURF_VM_STATE_SAVING);
}

/** @brief Returns whether the given VM has been saved, not running.
 *  @ingroup msg_VMs
 */
int MSG_vm_is_saved(msg_vm_t vm)
{
  return __MSG_vm_is_state(vm, SURF_VM_STATE_SAVED);
}

/** @brief Returns whether the given VM is being restored, not running.
 *  @ingroup msg_VMs
 */
int MSG_vm_is_restoring(msg_vm_t vm)
{
  return __MSG_vm_is_state(vm, SURF_VM_STATE_RESTORING);
}



/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

/* **** ******** MSG vm actions ********* **** */

/** @brief Create a new VM with specified parameters.
 *  @ingroup msg_VMs*
 *  All parameters are in MBytes
 *
 */
msg_vm_t MSG_vm_create(msg_host_t ind_pm, const char *name,
                       int ncpus, int ramsize,
                       int net_cap, char *disk_path, int disksize,
                       int mig_netspeed, int dp_intensity)
{
  /* For the moment, intensity_rate is the percentage against the migration
   * bandwidth */
  double host_speed = MSG_get_host_speed(ind_pm);
  double update_speed = ((double)dp_intensity/100) * mig_netspeed;

  msg_vm_t vm = MSG_vm_create_core(ind_pm, name);
  s_ws_params_t params;
  memset(&params, 0, sizeof(params));
  params.ramsize = (sg_size_t)ramsize * 1024 * 1024;
  //params.overcommit = 0;
  params.devsize = 0;
  params.skip_stage2 = 0;
  params.max_downtime = 0.03;
  params.dp_rate = (update_speed * 1024 * 1024) / host_speed;
  params.dp_cap = params.ramsize * 0.9; // assume working set memory is 90% of ramsize
  params.mig_speed = (double)mig_netspeed * 1024 * 1024; // mig_speed

  //XBT_INFO("dp rate %f migspeed : %f intensity mem : %d, updatespeed %f, hostspeed %f",params.dp_rate, params.mig_speed, dp_intensity, update_speed, host_speed);
  simcall_host_set_params(vm, &params);

  return vm;
}


/** @brief Create a new VM object. The VM is not yet started. The resource of the VM is allocated upon MSG_vm_start().
 *  @ingroup msg_VMs*
 *
 * A VM is treated as a host. The name of the VM must be unique among all hosts.
 */
msg_vm_t MSG_vm_create_core(msg_host_t ind_pm, const char *name)
{
  /* make sure the VM of the same name does not exit */
  {
    xbt_dictelm_t ind_host_tmp = xbt_lib_get_elm_or_null(host_lib, name);
    if (ind_host_tmp && xbt_lib_get_level(ind_host_tmp, SIMIX_HOST_LEVEL) != NULL) {
      XBT_ERROR("host %s already exits", name);
      return NULL;
    }
  }

  /* Note: ind_vm and vm_workstation point to the same elm object. */
  msg_vm_t ind_vm = NULL;
  void *ind_vm_workstation =  NULL;

  /* Ask the SIMIX layer to create the surf vm resource */
  ind_vm_workstation = simcall_vm_create(name, ind_pm);
  ind_vm = (msg_vm_t) __MSG_host_create(ind_vm_workstation);

  XBT_DEBUG("A new VM (%s) has been created", name);

  #ifdef HAVE_TRACING
  TRACE_msg_vm_create(name, ind_pm);
  #endif

  return ind_vm;
}

/** @brief Destroy a VM. Destroy the VM object from the simulation.
 *  @ingroup msg_VMs
 */
void MSG_vm_destroy(msg_vm_t vm)
{
  if (MSG_vm_is_migrating(vm))
    THROWF(vm_error, 0, "VM(%s) is migrating", sg_host_name(vm));

  /* First, terminate all processes on the VM if necessary */
  if (MSG_vm_is_running(vm))
      simcall_vm_shutdown(vm);

  if (!MSG_vm_is_created(vm)) {
    XBT_CRITICAL("shutdown the given VM before destroying it");
    DIE_IMPOSSIBLE;
  }

  /* Then, destroy the VM object */
  simcall_vm_destroy(vm);

  __MSG_host_destroy(vm);

  #ifdef HAVE_TRACING
  TRACE_msg_vm_end(vm);
  #endif
}


/** @brief Start a vm (i.e., boot the guest operating system)
 *  @ingroup msg_VMs
 *
 *  If the VM cannot be started, an exception is generated.
 *
 */
void MSG_vm_start(msg_vm_t vm)
{
  simcall_vm_start(vm);

  #ifdef HAVE_TRACING
  TRACE_msg_vm_start(vm);
  #endif
}



/** @brief Immediately kills all processes within the given VM. Any memory that they allocated will be leaked.
 *  @ingroup msg_VMs
 *
 * FIXME: No extra delay occurs. If you want to simulate this too, you want to
 * use a #MSG_process_sleep() or something. I'm not quite sure.
 */
void MSG_vm_shutdown(msg_vm_t vm)
{
  /* msg_vm_t equals to msg_host_t */
  simcall_vm_shutdown(vm);

  // #ifdef HAVE_TRACING
  // TRACE_msg_vm_(vm);
  // #endif
}



/* We have two mailboxes. mbox is used to transfer migration data between
 * source and destination PMs. mbox_ctl is used to detect the completion of a
 * migration. The names of these mailboxes must not conflict with others. */
static inline char *get_mig_mbox_src_dst(msg_vm_t vm, msg_host_t src_pm, msg_host_t dst_pm)
{
  char *vm_name = sg_host_name(vm);
  char *src_pm_name = sg_host_name(src_pm);
  char *dst_pm_name = sg_host_name(dst_pm);

  return bprintf("__mbox_mig_src_dst:%s(%s-%s)", vm_name, src_pm_name, dst_pm_name);
}

static inline char *get_mig_mbox_ctl(msg_vm_t vm, msg_host_t src_pm, msg_host_t dst_pm)
{
  char *vm_name = sg_host_name(vm);
  char *src_pm_name = sg_host_name(src_pm);
  char *dst_pm_name = sg_host_name(dst_pm);

  return bprintf("__mbox_mig_ctl:%s(%s-%s)", vm_name, src_pm_name, dst_pm_name);
}

static inline char *get_mig_process_tx_name(msg_vm_t vm, msg_host_t src_pm, msg_host_t dst_pm)
{
  char *vm_name = sg_host_name(vm);
  char *src_pm_name = sg_host_name(src_pm);
  char *dst_pm_name = sg_host_name(dst_pm);

  return bprintf("__pr_mig_tx:%s(%s-%s)", vm_name, src_pm_name, dst_pm_name);
}

static inline char *get_mig_process_rx_name(msg_vm_t vm, msg_host_t src_pm, msg_host_t dst_pm)
{
  char *vm_name = sg_host_name(vm);
  char *src_pm_name = sg_host_name(src_pm);
  char *dst_pm_name = sg_host_name(dst_pm);

  return bprintf("__pr_mig_rx:%s(%s-%s)", vm_name, src_pm_name, dst_pm_name);
}

static inline char *get_mig_task_name(msg_vm_t vm, msg_host_t src_pm, msg_host_t dst_pm, int stage)
{
  char *vm_name = sg_host_name(vm);
  char *src_pm_name = sg_host_name(src_pm);
  char *dst_pm_name = sg_host_name(dst_pm);

  return bprintf("__task_mig_stage%d:%s(%s-%s)", stage, vm_name, src_pm_name, dst_pm_name);
}

static void launch_deferred_exec_process(msg_host_t host, double computation, double prio);


struct migration_session {
  msg_vm_t vm;
  msg_host_t src_pm;
  msg_host_t dst_pm;

  /* The miration_rx process uses mbox_ctl to let the caller of do_migration()
   * know the completion of the migration. */
  char *mbox_ctl;
  /* The migration_rx and migration_tx processes use mbox to transfer migration
   * data. */
  char *mbox;
};


static int migration_rx_fun(int argc, char *argv[])
{
  XBT_DEBUG("mig: rx_start");

  // The structure has been created in the do_migration function and should only be freed in the same place ;)
  struct migration_session *ms = MSG_process_get_data(MSG_process_self());

  s_ws_params_t params;
  simcall_host_get_params(ms->vm, &params);
  const double xfer_cpu_overhead = params.xfer_cpu_overhead;

  int need_exit = 0;

  char *finalize_task_name = get_mig_task_name(ms->vm, ms->src_pm, ms->dst_pm, 3);

  int ret = 0; 
  for (;;) {
    msg_task_t task = NULL;
    ret = MSG_task_recv(&task, ms->mbox);
    {
      double received ;
      if (ret == MSG_OK)
      	received = MSG_task_get_data_size(task);
      else{
	// An error occured, clean the code and return
        // The owner did not change, hence the task should be only destroyed on the other side
        xbt_free(finalize_task_name);
         return 0;
      }
      /* TODO: clean up */
      // const double alpha = 0.22L * 1.0E8 / (80L * 1024 * 1024);
      launch_deferred_exec_process(ms->vm, received * xfer_cpu_overhead, 1);
    }

    if (strcmp(task->name, finalize_task_name) == 0)
      need_exit = 1;

    MSG_task_destroy(task);

    if (need_exit)
      break;
  }

  // Here Stage 1, 2  and 3 have been performed. 
  // Hence complete the migration

  // Copy the reference to the vm (if SRC crashes now, do_migration will free ms)
  // This is clearly ugly but I (Adrien) need more time to do something cleaner (actually we should copy the whole ms structure at the begining and free it at the end of each function)
   msg_vm_t vm = ms->vm; 
   msg_host_t src_pm = ms->src_pm; 
   msg_host_t dst_pm = ms-> dst_pm; 
   msg_host_priv_t priv = msg_host_resource_priv(vm);

// TODO: we have an issue, if the DST node is turning off during the three next calls, then the VM is in an inconsistent state
// I should check with Takahiro in order to make this portion of code atomic
  /* deinstall the current affinity setting for the CPU */
  simcall_vm_set_affinity(vm, src_pm, 0);

  /* Update the vm location */
  simcall_vm_migrate(vm, dst_pm);
  
  /* Resume the VM */
  simcall_vm_resume(vm);

  /* install the affinity setting of the VM on the destination pm */
  {

    unsigned long affinity_mask = (unsigned long) xbt_dict_get_or_null_ext(priv->affinity_mask_db, (char *)dst_pm, sizeof(msg_host_t));
    simcall_vm_set_affinity(vm, dst_pm, affinity_mask);
    XBT_DEBUG("set affinity(0x%04lx@%s) for %s", affinity_mask, MSG_host_get_name(dst_pm), MSG_host_get_name(vm));
  }

  {

   // Now the VM is running on the new host (the migration is completed) (even if the SRC crash)
   msg_host_priv_t priv = msg_host_resource_priv(vm);
   priv->is_migrating = 0;
   XBT_DEBUG("VM(%s) moved from PM(%s) to PM(%s)", ms->vm->key, ms->src_pm->key, ms->dst_pm->key);
   #ifdef HAVE_TRACING
    TRACE_msg_vm_change_host(ms->vm, ms->src_pm, ms->dst_pm);
   #endif
  
  }
  // Inform the SRC that the migration has been correctly performed
  {
    char *task_name = get_mig_task_name(ms->vm, ms->src_pm, ms->dst_pm, 4);
    msg_task_t task = MSG_task_create(task_name, 0, 0, NULL);
    msg_error_t ret = MSG_task_send(task, ms->mbox_ctl);
    // xbt_assert(ret == MSG_OK);
    if(ret == MSG_HOST_FAILURE){
    // The DST has crashed, this is a problem has the VM since we are not sure whether SRC is considering that the VM has been correctly migrated on the DST node
    // TODO What does it mean ? What should we do ? 
     MSG_task_destroy(task);
    } else if(ret == MSG_TRANSFER_FAILURE){
    // The SRC has crashed, this is not a problem has the VM has been correctly migrated on the DST node
 	MSG_task_destroy(task);
     }

    xbt_free(task_name);
  }


  xbt_free(finalize_task_name);

  XBT_DEBUG("mig: rx_done");

  return 0;
}

static void reset_dirty_pages(msg_vm_t vm)
{
  msg_host_priv_t priv = msg_host_resource_priv(vm);

  char *key = NULL;
  xbt_dict_cursor_t cursor = NULL;
  dirty_page_t dp = NULL;
  xbt_dict_foreach(priv->dp_objs, cursor, key, dp) {
    double remaining = MSG_task_get_remaining_computation(dp->task);
    dp->prev_clock = MSG_get_clock();
    dp->prev_remaining = remaining;

    // XBT_INFO("%s@%s remaining %f", key, sg_host_name(vm), remaining);
  }
}

static void start_dirty_page_tracking(msg_vm_t vm)
{
  msg_host_priv_t priv = msg_host_resource_priv(vm);
  priv->dp_enabled = 1;

  reset_dirty_pages(vm);
}

static void stop_dirty_page_tracking(msg_vm_t vm)
{
  msg_host_priv_t priv = msg_host_resource_priv(vm);
  priv->dp_enabled = 0;
}

#if 0
/* It might be natural that we define dp_rate for each task. But, we will also
 * have to care about how each task behavior affects the memory update behavior
 * at the operating system level. It may not be easy to model it with a simple algorithm. */
double calc_updated_pages(char *key, msg_vm_t vm, dirty_page_t dp, double remaining, double clock)
{
    double computed = dp->prev_remaining - remaining;
    double duration = clock - dp->prev_clock;
    double updated = dp->task->dp_rate * computed;

    XBT_INFO("%s@%s: computated %f ops (remaining %f -> %f) in %f secs (%f -> %f)",
        key, sg_host_name(vm), computed, dp->prev_remaining, remaining, duration, dp->prev_clock, clock);
    XBT_INFO("%s@%s: updated %f bytes, %f Mbytes/s",
        key, sg_host_name(vm), updated, updated / duration / 1000 / 1000);

    return updated;
}
#endif

static double get_computed(char *key, msg_vm_t vm, dirty_page_t dp, double remaining, double clock)
{
  double computed = dp->prev_remaining - remaining;
  double duration = clock - dp->prev_clock;

  XBT_DEBUG("%s@%s: computed %f ops (remaining %f -> %f) in %f secs (%f -> %f)",
      key, sg_host_name(vm), computed, dp->prev_remaining, remaining, duration, dp->prev_clock, clock);

  return computed;
}

static double lookup_computed_flop_counts(msg_vm_t vm, int stage_for_fancy_debug, int stage2_round_for_fancy_debug)
{
  msg_host_priv_t priv = msg_host_resource_priv(vm);
  double total = 0;

  char *key = NULL;
  xbt_dict_cursor_t cursor = NULL;
  dirty_page_t dp = NULL;
  xbt_dict_foreach(priv->dp_objs, cursor, key, dp) {
    double remaining = MSG_task_get_remaining_computation(dp->task);

	 double clock = MSG_get_clock();

    // total += calc_updated_pages(key, vm, dp, remaining, clock);
    total += get_computed(key, vm, dp, remaining, clock);

    dp->prev_remaining = remaining;
    dp->prev_clock = clock;
  }

  total += priv->dp_updated_by_deleted_tasks;

  XBT_DEBUG("mig-stage%d.%d: computed %f flop_counts (including %f by deleted tasks)",
      stage_for_fancy_debug,
      stage2_round_for_fancy_debug,
      total, priv->dp_updated_by_deleted_tasks);



  priv->dp_updated_by_deleted_tasks = 0;


  return total;
}

// TODO Is this code redundant with the information provided by
// msg_process_t MSG_process_create(const char *name, xbt_main_func_t code, void *data, msg_host_t host)
void MSG_host_add_task(msg_host_t host, msg_task_t task)
{
  msg_host_priv_t priv = msg_host_resource_priv(host);
  double remaining = MSG_task_get_remaining_computation(task);
  char *key = bprintf("%s-%p", task->name, task);

  dirty_page_t dp = xbt_new0(s_dirty_page, 1);
  dp->task = task;

  /* It should be okay that we add a task onto a migrating VM. */
  if (priv->dp_enabled) {
    dp->prev_clock = MSG_get_clock();
    dp->prev_remaining = remaining;
  }

  xbt_assert(xbt_dict_get_or_null(priv->dp_objs, key) == NULL);
  xbt_dict_set(priv->dp_objs, key, dp, NULL);
  XBT_DEBUG("add %s on %s (remaining %f, dp_enabled %d)", key, sg_host_name(host), remaining, priv->dp_enabled);

  xbt_free(key);
}

void MSG_host_del_task(msg_host_t host, msg_task_t task)
{
  msg_host_priv_t priv = msg_host_resource_priv(host);

  char *key = bprintf("%s-%p", task->name, task);

  dirty_page_t dp = xbt_dict_get_or_null(priv->dp_objs, key);
  xbt_assert(dp->task == task);

  /* If we are in the middle of dirty page tracking, we record how much
   * computation has been done until now, and keep the information for the
   * lookup_() function that will called soon. */
  if (priv->dp_enabled) {
    double remaining = MSG_task_get_remaining_computation(task);
    double clock = MSG_get_clock();
    // double updated = calc_updated_pages(key, host, dp, remaining, clock);
    double updated = get_computed(key, host, dp, remaining, clock);

    priv->dp_updated_by_deleted_tasks += updated;
  }

  xbt_dict_remove(priv->dp_objs, key);
  xbt_free(dp);

  XBT_DEBUG("del %s on %s", key, sg_host_name(host));

  xbt_free(key);
}


static int deferred_exec_fun(int argc, char *argv[])
{
  xbt_assert(argc == 3);
  const char *comp_str = argv[1];
  double computaion = atof(comp_str);
  const char *prio_str = argv[2];
  double prio = atof(prio_str);

  msg_task_t task = MSG_task_create("__task_deferred", computaion, 0, NULL);
  // XBT_INFO("exec deferred %f", computation);

  /* dpt is the results of the VM activity */
  MSG_task_set_priority(task, prio);
  MSG_task_execute(task);



  MSG_task_destroy(task);

  return 0;
}

static void launch_deferred_exec_process(msg_host_t host, double computation, double prio)
{
  char *pr_name = bprintf("__pr_deferred_exec_%s", MSG_host_get_name(host));

  int nargvs = 4;
  char **argv = xbt_new(char *, nargvs);
  argv[0] = pr_name;
  argv[1] = bprintf("%f", computation);
  argv[2] = bprintf("%f", prio);
  argv[3] = NULL;

  MSG_process_create_with_arguments(pr_name, deferred_exec_fun, NULL, host, nargvs - 1, argv);
}


static int task_tx_overhead_fun(int argc, char *argv[])
{
  xbt_assert(argc == 2);
  const char *mbox = argv[1];

  int need_exit = 0;

  // XBT_INFO("start %s", mbox);

  for (;;) {
    msg_task_t task = NULL;
    MSG_task_recv(&task, mbox);

    // XBT_INFO("task->name %s", task->name);

    if (strcmp(task->name, "finalize_making_overhead") == 0)
      need_exit = 1;

    // XBT_INFO("exec");
    // MSG_task_set_priority(task, 1000000);
    MSG_task_execute(task);
    MSG_task_destroy(task);

    if (need_exit)
      break;
  }

  // XBT_INFO("bye");

  return 0;
}

static void start_overhead_process(msg_task_t comm_task)
{
  char *pr_name = bprintf("__pr_task_tx_overhead_%s", MSG_task_get_name(comm_task));
  char *mbox    = bprintf("__mb_task_tx_overhead_%s", MSG_task_get_name(comm_task));

  int nargvs = 3;
  char **argv = xbt_new(char *, nargvs);
  argv[0] = pr_name;
  argv[1] = mbox;
  argv[2] = NULL;

  // XBT_INFO("micro start: mbox %s", mbox);
  MSG_process_create_with_arguments(pr_name, task_tx_overhead_fun, NULL, MSG_host_self(), nargvs - 1, argv);
}

static void shutdown_overhead_process(msg_task_t comm_task)
{
  char *mbox = bprintf("__mb_task_tx_overhead_%s", MSG_task_get_name(comm_task));

  msg_task_t task = MSG_task_create("finalize_making_overhead", 0, 0, NULL);

  // XBT_INFO("micro shutdown: mbox %s", mbox);
  msg_error_t ret = MSG_task_send(task, mbox);
  if(ret != MSG_OK)
    xbt_die("shutdown error - task not sent");

  xbt_free(mbox);
  // XBT_INFO("shutdown done");
}

static void request_overhead(msg_task_t comm_task, double computation)
{
  char *mbox = bprintf("__mb_task_tx_overhead_%s", MSG_task_get_name(comm_task));

  msg_task_t task = MSG_task_create("micro", computation, 0, NULL);

  // XBT_INFO("req overhead");
  msg_error_t ret = MSG_task_send(task, mbox);
  if(ret != MSG_OK)
    xbt_die("req overhead error - task not sent");

  xbt_free(mbox);
}

/* alpha is (floating_operations / bytes).
 *
 * When actual migration traffic was 32 mbytes/s, we observed the CPU
 * utilization of the main thread of the Qemu process was 10 %. 
 *   alpha = 0.1 * C / (32 * 1024 * 1024)
 * where the CPU capacity of the PM is C flops/s.
 *
 * */
static void task_send_bounded_with_cpu_overhead(msg_task_t comm_task, char *mbox, double mig_speed, double alpha)
{
  const double chunk_size = 1024 * 1024 * 10;
  double remaining = MSG_task_get_data_size(comm_task);

  start_overhead_process(comm_task);


  while (remaining > 0) {
    double data_size = chunk_size;
    if (remaining < chunk_size)
      data_size = remaining;

    remaining -= data_size;

    // XBT_INFO("remaining %f bytes", remaining);


    double clock_sta = MSG_get_clock();

    /* create a micro task */
    {
      char *mtask_name = bprintf("__micro_%s", MSG_task_get_name(comm_task));
      msg_task_t mtask = MSG_task_create(mtask_name, 0, data_size, NULL);

      request_overhead(comm_task, data_size * alpha);

      msg_error_t ret = MSG_task_send(mtask, mbox);
      if(ret != MSG_OK)
        xbt_die("migration error - task not sent");

      xbt_free(mtask_name);
    }

#if 0
    {
      /* In the real world, sending data involves small CPU computation. */
      char *mtask_name = bprintf("__micro_%s", MSG_task_get_name(comm_task));
      msg_task_t mtask = MSG_task_create(mtask_name, data_size * alpha, data_size, NULL);
      MSG_task_execute(mtask);
      MSG_task_destroy(mtask);
      xbt_free(mtask_name);
    }
#endif
   
    /* TODO */

    double clock_end = MSG_get_clock();


    if (mig_speed > 0) {
      /*
       * (max bandwidth) > data_size / ((elapsed time) + time_to_sleep)
       *
       * Thus, we get
       *   time_to_sleep > data_size / (max bandwidth) - (elapsed time)
       *
       * If time_to_sleep is smaller than zero, the elapsed time was too big. We
       * do not need a micro sleep.
       **/
      double time_to_sleep = data_size / mig_speed - (clock_end - clock_sta);
      if (time_to_sleep > 0)
        MSG_process_sleep(time_to_sleep);


      //XBT_INFO("duration %f", clock_end - clock_sta);
      //XBT_INFO("time_to_sleep %f", time_to_sleep);
    }
  }

  // XBT_INFO("%s", MSG_task_get_name(comm_task));
  shutdown_overhead_process(comm_task);

}


#if 0
static void make_cpu_overhead_of_data_transfer(msg_task_t comm_task, double init_comm_size)
{
  double prev_remaining = init_comm_size;

  for (;;) {
    double remaining = MSG_task_get_remaining_communication(comm_task);
    if (remaining == 0)
      need_exit = 1;

    double sent = prev_remaining - remaining;
    double comp_size = sent * overhead;


    char *comp_task_name = bprintf("__sender_overhead%s", MSG_task_get_name(comm_task));
    msg_task_t comp_task = MSG_task_create(comp_task_name, comp_size, 0, NULL);
    MSG_task_execute(comp_task);
    MSG_task_destroy(comp_task);

    if (need_exit)
      break;

    prev_remaining = remaining;

  }

  xbt_free(comp_task_name);
}
#endif

// #define USE_MICRO_TASK 1

#if 0
// const double alpha = 0.1L * 1.0E8 / (32L * 1024 * 1024);
// const double alpha = 0.25L * 1.0E8 / (85L * 1024 * 1024);
// const double alpha = 0.20L * 1.0E8 / (85L * 1024 * 1024);
// const double alpha = 0.25L * 1.0E8 / (85L * 1024 * 1024);
// const double alpha = 0.32L * 1.0E8 / (24L * 1024 * 1024);   // makes super good values for 32 mbytes/s
//const double alpha = 0.32L * 1.0E8 / (32L * 1024 * 1024);
// const double alpha = 0.56L * 1.0E8 / (80L * 1024 * 1024);
////const double alpha = 0.20L * 1.0E8 / (80L * 1024 * 1024);
// const double alpha = 0.56L * 1.0E8 / (90L * 1024 * 1024);
// const double alpha = 0.66L * 1.0E8 / (90L * 1024 * 1024);
// const double alpha = 0.20L * 1.0E8 / (80L * 1024 * 1024);

/* CPU 22% when 80Mbyte/s */
const double alpha = 0.22L * 1.0E8 / (80L * 1024 * 1024);
#endif


static void send_migration_data(msg_vm_t vm, msg_host_t src_pm, msg_host_t dst_pm,
    sg_size_t size, char *mbox, int stage, int stage2_round, double mig_speed, double xfer_cpu_overhead)
{
  char *task_name = get_mig_task_name(vm, src_pm, dst_pm, stage);
  msg_task_t task = MSG_task_create(task_name, 0, size, NULL);

  /* TODO: clean up */

  double clock_sta = MSG_get_clock();

#ifdef USE_MICRO_TASK

  task_send_bounded_with_cpu_overhead(task, mbox, mig_speed, xfer_cpu_overhead);

#else
  msg_error_t ret;
  if (mig_speed > 0)
    ret = MSG_task_send_bounded(task, mbox, mig_speed);
  else
    ret = MSG_task_send(task, mbox);

//  xbt_assert(ret == MSG_OK);
  xbt_free(task_name);
  if(ret == MSG_HOST_FAILURE){
	//XBT_INFO("SRC host failed during migration of %s (stage %d)", sg_host_name(vm), stage);
 	MSG_task_destroy(task);
	THROWF(host_error, 0, "SRC host failed during migration of %s (stage %d)", sg_host_name(vm), stage);
   }else if(ret == MSG_TRANSFER_FAILURE){
	//XBT_INFO("DST host failed during migration of %s (stage %d)", sg_host_name(vm), stage);
 	MSG_task_destroy(task);
	THROWF(host_error, 0, "DST host failed during migration of %s (stage %d)", sg_host_name(vm), stage);
  }
//else
//   XBT_INFO("Ret != FAILURE !!!!"); 
#endif

  double clock_end = MSG_get_clock();
  double duration = clock_end - clock_sta;
  double actual_speed = size / duration;
#ifdef USE_MICRO_TASK
  double cpu_utilization = size * xfer_cpu_overhead / duration / 1.0E8;
#else
  double cpu_utilization = 0;
#endif

  if (stage == 2){
    XBT_DEBUG("mig-stage%d.%d: sent %llu duration %f actual_speed %f (target %f) cpu %f", stage, stage2_round, size, duration, actual_speed, mig_speed, cpu_utilization);}
  else{
    XBT_DEBUG("mig-stage%d: sent %llu duration %f actual_speed %f (target %f) cpu %f", stage, size, duration, actual_speed, mig_speed, cpu_utilization);
  }


#ifdef USE_MICRO_TASK
  /* The name of a micro task starts with __micro, which does not match the
   * special name that finalizes the receiver loop. Thus, we send the special task.
   **/
  {
    if (stage == 3) {
      char *task_name = get_mig_task_name(vm_name, src_pm_name, dst_pm_name, stage);
      msg_task_t task = MSG_task_create(task_name, 0, 0, NULL);
      msg_error_t ret = MSG_task_send(task, mbox);
//      xbt_assert(ret == MSG_OK);
      xbt_free(task_name);
      if(ret == MSG_HOST_FAILURE){
	//XBT_INFO("SRC host failed during migration of %s (stage 3)", sg_host_name(vm));
 	MSG_task_destroy(task);
	THROWF(host_error, 0, "SRC host failed during migration of VM %s (stage 3)", sg_host_name(vm));
        // The owner of the task did not change so destroy the task 
	return; 
      }else if(ret == MSG_TRANSFER_FAILURE){
	//XBT_INFO("DST host failed during migration of %s (stage %d)", sg_host_name(vm), stage);
 	MSG_task_destroy(task);
	THROWF(host_error, 0, "DST host failed during migration of %s (stage %d)", sg_host_name(vm), stage);
	return; 
     }

    }
  }
#endif

}

static double get_updated_size(double computed, double dp_rate, double dp_cap)
{
  double updated_size = computed * dp_rate;
  XBT_DEBUG("updated_size %f dp_rate %f", updated_size, dp_rate);
  if (updated_size > dp_cap) {
    // XBT_INFO("mig-stage2.%d: %f bytes updated, but cap it with the working set size %f", stage2_round, updated_size, dp_cap);
    updated_size = dp_cap;
  }

  return updated_size;
}

static double send_stage1(struct migration_session *ms,
    sg_size_t ramsize, double mig_speed, double xfer_cpu_overhead, double dp_rate, double dp_cap, double dpt_cpu_overhead)
{

  // const long chunksize = (sg_size_t)1024 * 1024 * 100;
  const sg_size_t chunksize = (sg_size_t)1024 * 1024 * 100000;
  sg_size_t remaining = ramsize;
  double computed_total = 0;

  while (remaining > 0) {
    sg_size_t datasize = chunksize;
    if (remaining < chunksize)
      datasize = remaining;

    remaining -= datasize;
    send_migration_data(ms->vm, ms->src_pm, ms->dst_pm, datasize, ms->mbox, 1, 0, mig_speed, xfer_cpu_overhead);
    double computed = lookup_computed_flop_counts(ms->vm, 1, 0);
    computed_total += computed;

    // {
    //   double updated_size = get_updated_size(computed, dp_rate, dp_cap);

    //   double overhead = dpt_cpu_overhead * updated_size;
    //   launch_deferred_exec_process(vm, overhead, 10000);
    // }
  }

  return computed_total;
}



static double get_threshold_value(double bandwidth, double max_downtime)
{
  /* This value assumes the network link is 1Gbps. */
  // double threshold = max_downtime * 125 * 1024 * 1024;
  double threshold = max_downtime * bandwidth;

  return threshold;
}

static int migration_tx_fun(int argc, char *argv[])
{
  XBT_DEBUG("mig: tx_start");

  // Note that the ms structure has been allocated in do_migration and hence should be freed in the same function ;) 
  struct migration_session *ms = MSG_process_get_data(MSG_process_self());

  s_ws_params_t params;
  simcall_host_get_params(ms->vm, &params);
  const sg_size_t ramsize   = params.ramsize;
  const sg_size_t devsize   = params.devsize;
  const int skip_stage1     = params.skip_stage1;
  const int skip_stage2     = params.skip_stage2;
  const double dp_rate      = params.dp_rate;
  const double dp_cap       = params.dp_cap;
  const double mig_speed    = params.mig_speed;
  const double xfer_cpu_overhead = params.xfer_cpu_overhead;
  const double dpt_cpu_overhead = params.dpt_cpu_overhead;

  msg_vm_t vm=ms->vm; 

  double remaining_size = ramsize + devsize;

  double max_downtime = params.max_downtime;
  if (max_downtime == 0) {
    XBT_WARN("use the default max_downtime value 30ms");
    max_downtime = 0.03;
  }

  double threshold = 0.00001; /* TODO: cleanup */

  /* setting up parameters has done */


  if (ramsize == 0)
    XBT_WARN("migrate a VM, but ramsize is zero");


  XBT_DEBUG("mig-stage1: remaining_size %f", remaining_size);

  /* Stage1: send all memory pages to the destination. */
  start_dirty_page_tracking(vm);

  double computed_during_stage1 = 0;
  if (!skip_stage1) {
    // send_migration_data(vm_name, src_pm_name, dst_pm_name, ramsize, mbox, 1, 0, mig_speed, xfer_cpu_overhead);

    /* send ramsize, but split it */
    double clock_prev_send = MSG_get_clock();

    TRY{
    	computed_during_stage1 = send_stage1(ms, ramsize, mig_speed, xfer_cpu_overhead, dp_rate, dp_cap, dpt_cpu_overhead);
    } CATCH_ANONYMOUS{
      //hostfailure (if you want to know whether this is the SRC or the DST please check directly in send_migration_data code)
      // Stop the dirty page tracking an return (there is no memory space to release) 
      stop_dirty_page_tracking(vm);
      return 0; 
    }
    remaining_size -= ramsize;

    double clock_post_send = MSG_get_clock();
    double bandwidth = ramsize / (clock_post_send - clock_prev_send);
    threshold = get_threshold_value(bandwidth, max_downtime);
    XBT_DEBUG("actual bandwidth %f (MB/s), threshold %f", bandwidth / 1024 / 1024, threshold);
  }


  /* Stage2: send update pages iteratively until the size of remaining states
   * becomes smaller than the threshold value. */
  if (skip_stage2)
    goto stage3;
  if (max_downtime == 0) {
    XBT_WARN("no max_downtime parameter, skip stage2");
    goto stage3;
  }


  int stage2_round = 0;
  for (;;) {

    double updated_size = 0;
    if (stage2_round == 0)  {
      /* just after stage1, nothing has been updated. But, we have to send the data updated during stage1 */
      updated_size = get_updated_size(computed_during_stage1, dp_rate, dp_cap);
    } else {
      double computed = lookup_computed_flop_counts(ms->vm, 2, stage2_round);
      updated_size = get_updated_size(computed, dp_rate, dp_cap);
    }

    XBT_DEBUG("mig-stage 2:%d updated_size %f computed_during_stage1 %f dp_rate %f dp_cap %f",
        stage2_round, updated_size, computed_during_stage1, dp_rate, dp_cap);


    // if (stage2_round != 0) {
    //   /* during stage1, we have already created overhead tasks */
    //   double overhead = dpt_cpu_overhead * updated_size;
    //   XBT_DEBUG("updated %f overhead %f", updated_size, overhead);
    //   launch_deferred_exec_process(vm, overhead, 10000);
    // }


    {
      remaining_size += updated_size;

      XBT_DEBUG("mig-stage2.%d: remaining_size %f (%s threshold %f)", stage2_round,
          remaining_size, (remaining_size < threshold) ? "<" : ">", threshold);

      if (remaining_size < threshold)
        break;
    }

    double clock_prev_send = MSG_get_clock();
    TRY{
      send_migration_data(ms->vm, ms->src_pm, ms->dst_pm, updated_size, ms->mbox, 2, stage2_round, mig_speed, xfer_cpu_overhead);
    }CATCH_ANONYMOUS{
      //hostfailure (if you want to know whether this is the SRC or the DST please check directly in send_migration_data code)
      // Stop the dirty page tracking an return (there is no memory space to release) 
      stop_dirty_page_tracking(vm);
      return 0; 
    }
    double clock_post_send = MSG_get_clock();

    double bandwidth = updated_size / (clock_post_send - clock_prev_send);
    threshold = get_threshold_value(bandwidth, max_downtime);
    XBT_DEBUG("actual bandwidth %f, threshold %f", bandwidth / 1024 / 1024, threshold);


    remaining_size -= updated_size;
    stage2_round += 1;
  }


stage3:
  /* Stage3: stop the VM and copy the rest of states. */
  XBT_DEBUG("mig-stage3: remaining_size %f", remaining_size);
  simcall_vm_suspend(vm);
  stop_dirty_page_tracking(vm);
 
 TRY{
    send_migration_data(ms->vm, ms->src_pm, ms->dst_pm, remaining_size, ms->mbox, 3, 0, mig_speed, xfer_cpu_overhead);
  }CATCH_ANONYMOUS{
      //hostfailure (if you want to know whether this is the SRC or the DST please check directly in send_migration_data code)
      // Stop the dirty page tracking an return (there is no memory space to release) 
      simcall_vm_resume(vm);
      return 0; 
    }
  
 // At that point the Migration is considered valid for the SRC node but remind that the DST side should relocate effectively the VM on the DST node. 

  XBT_DEBUG("mig: tx_done");

  return 0;
}



static int do_migration(msg_vm_t vm, msg_host_t src_pm, msg_host_t dst_pm)
{
  struct migration_session *ms = xbt_new(struct migration_session, 1);
  ms->vm = vm;
  ms->src_pm = src_pm;
  ms->dst_pm = dst_pm;
  ms->mbox_ctl = get_mig_mbox_ctl(vm, src_pm, dst_pm);
  ms->mbox = get_mig_mbox_src_dst(vm, src_pm, dst_pm);
  

  char *pr_rx_name = get_mig_process_rx_name(vm, src_pm, dst_pm);
  char *pr_tx_name = get_mig_process_tx_name(vm, src_pm, dst_pm);

//  MSG_process_create(pr_rx_name, migration_rx_fun, ms, dst_pm);
//  MSG_process_create(pr_tx_name, migration_tx_fun, ms, src_pm);
#if 1
 {
 char **argv = xbt_new(char *, 2);
 argv[0] = pr_rx_name;
 argv[1] = NULL;
 MSG_process_create_with_arguments(pr_rx_name, migration_rx_fun, ms, dst_pm, 1, argv);
 }
 {
 char **argv = xbt_new(char *, 2);
 argv[0] = pr_tx_name;
 argv[1] = NULL;
 MSG_process_create_with_arguments(pr_tx_name, migration_tx_fun, ms, src_pm, 1, argv);
 }
#endif

  /* wait until the migration have finished or on error has occured */
  {
    XBT_DEBUG("wait for reception of the final ACK (i.e. migration has been correctly performed");
    msg_task_t task = NULL;
    msg_error_t ret = MSG_TIMEOUT; 
    while (ret == MSG_TIMEOUT && MSG_host_is_on(dst_pm)) //Wait while you receive the message o
     ret = MSG_task_receive_with_timeout(&task, ms->mbox_ctl, 10);

    xbt_free(ms->mbox_ctl);
    xbt_free(ms->mbox);
    xbt_free(ms);
    
    //xbt_assert(ret == MSG_OK);
    if(ret == MSG_HOST_FAILURE){
        // Note that since the communication failed, the owner did not change and the task should be destroyed on the other side.
        // Hence, just throw the execption
        //XBT_INFO("SRC crashes, throw an exception (m-control)");
        return -1; 
    } 
    else if((ret == MSG_TRANSFER_FAILURE) || (ret == MSG_TIMEOUT)){ // MSG_TIMEOUT here means that MSG_host_is_avail() returned false.
        //XBT_INFO("DST crashes, throw an exception (m-control)");
        return -2;  
    }

    
    char *expected_task_name = get_mig_task_name(vm, src_pm, dst_pm, 4);
    xbt_assert(strcmp(task->name, expected_task_name) == 0);
    xbt_free(expected_task_name);
    MSG_task_destroy(task);
    return 0; 
  }
}




/** @brief Migrate the VM to the given host.
 *  @ingroup msg_VMs
 *
 * FIXME: No migration cost occurs. If you want to simulate this too, you want to use a
 * MSG_task_send() before or after, depending on whether you want to do cold or hot
 * migration.
 */
void MSG_vm_migrate(msg_vm_t vm, msg_host_t new_pm)
{
  /* some thoughts:
   * - One approach is ...
   *   We first create a new VM (i.e., destination VM) on the destination
   *   physical host. The destination VM will receive the state of the source
   *   VM over network. We will finally destroy the source VM.
   *   - This behavior is similar to the way of migration in the real world.
   *     Even before a migration is completed, we will see a destination VM,
   *     consuming resources.
   *   - We have to relocate all processes. The existing process migraion code
   *     will work for this?
   *   - The name of the VM is a somewhat unique ID in the code. It is tricky
   *     for the destination VM?
   *
   * - Another one is ...
   *   We update the information of the given VM to place it to the destination
   *   physical host.
   *
   * The second one would be easier.
   *   
   */

  msg_host_t old_pm = simcall_vm_get_pm(vm);

  if (!MSG_vm_is_running(vm))
    THROWF(vm_error, 0, "VM(%s) is not running", sg_host_name(vm));

  if (MSG_vm_is_migrating(vm))
    THROWF(vm_error, 0, "VM(%s) is already migrating", sg_host_name(vm));

  msg_host_priv_t priv = msg_host_resource_priv(vm);
  priv->is_migrating = 1;

  {
   
    int ret = do_migration(vm, old_pm, new_pm); 
    if (ret == -1){
     priv->is_migrating = 0;
     THROWF(host_error, 0, "SRC host failed during migration");
    }
    else if(ret == -2){ 
     priv->is_migrating = 0;
     THROWF(host_error, 0, "DST host failed during migration");
    }
  }

  // This part is done in the RX code, to handle the corner case where SRC can crash just at the end of the migration process
  // In that case, the VM has been already assigned to the DST node.
  //XBT_DEBUG("VM(%s) moved from PM(%s) to PM(%s)", vm->key, old_pm->key, new_pm->key);
  //#ifdef HAVE_TRACING
  //TRACE_msg_vm_change_host(vm, old_pm, new_pm);
  //#endif
}


/** @brief Immediately suspend the execution of all processes within the given VM.
 *  @ingroup msg_VMs
 *
 * This function stops the execution of the VM. All the processes on this VM
 * will pause. The state of the VM is preserved. We can later resume it again.
 *
 * No suspension cost occurs.
 */
void MSG_vm_suspend(msg_vm_t vm)
{
  if (MSG_vm_is_migrating(vm))
    THROWF(vm_error, 0, "VM(%s) is migrating", sg_host_name(vm));

  simcall_vm_suspend(vm);

  XBT_DEBUG("vm_suspend done");

  #ifdef HAVE_TRACING
  TRACE_msg_vm_suspend(vm);
  #endif
}


/** @brief Resume the execution of the VM. All processes on the VM run again.
 *  @ingroup msg_VMs
 *
 * No resume cost occurs.
 */
void MSG_vm_resume(msg_vm_t vm)
{
  simcall_vm_resume(vm);

  #ifdef HAVE_TRACING
  TRACE_msg_vm_resume(vm);
  #endif
}


/** @brief Immediately save the execution of all processes within the given VM.
 *  @ingroup msg_VMs
 *
 * This function stops the execution of the VM. All the processes on this VM
 * will pause. The state of the VM is preserved. We can later resume it again.
 *
 * FIXME: No suspension cost occurs. If you want to simulate this too, you want to
 * use a \ref MSG_file_write() before or after, depending on the exact semantic
 * of VM save to you.
 */
void MSG_vm_save(msg_vm_t vm)
{
  if (MSG_vm_is_migrating(vm))
    THROWF(vm_error, 0, "VM(%s) is migrating", sg_host_name(vm));

  simcall_vm_save(vm);
  #ifdef HAVE_TRACING
  TRACE_msg_vm_save(vm);
  #endif
}

/** @brief Restore the execution of the VM. All processes on the VM run again.
 *  @ingroup msg_VMs
 *
 * FIXME: No restore cost occurs. If you want to simulate this too, you want to
 * use a \ref MSG_file_read() before or after, depending on the exact semantic
 * of VM restore to you.
 */
void MSG_vm_restore(msg_vm_t vm)
{
  simcall_vm_restore(vm);

  #ifdef HAVE_TRACING
  TRACE_msg_vm_restore(vm);
  #endif
}


/** @brief Get the physical host of a given VM.
 *  @ingroup msg_VMs
 */
msg_host_t MSG_vm_get_pm(msg_vm_t vm)
{
  return simcall_vm_get_pm(vm);
}


/** @brief Set a CPU bound for a given VM.
 *  @ingroup msg_VMs
 *
 * 1.
 * Note that in some cases MSG_task_set_bound() may not intuitively work for VMs.
 *
 * For example,
 *  On PM0, there are Task1 and VM0.
 *  On VM0, there is Task2.
 * Now we bound 75% to Task1\@PM0 and bound 25% to Task2\@VM0.
 * Then, 
 *  Task1\@PM0 gets 50%.
 *  Task2\@VM0 gets 25%.
 * This is NOT 75% for Task1\@PM0 and 25% for Task2\@VM0, respectively.
 *
 * This is because a VM has the dummy CPU action in the PM layer. Putting a
 * task on the VM does not affect the bound of the dummy CPU action. The bound
 * of the dummy CPU action is unlimited.
 *
 * There are some solutions for this problem. One option is to update the bound
 * of the dummy CPU action automatically. It should be the sum of all tasks on
 * the VM. But, this solution might be costly, because we have to scan all tasks
 * on the VM in share_resource() or we have to trap both the start and end of
 * task execution.
 *
 * The current solution is to use MSG_vm_set_bound(), which allows us to
 * directly set the bound of the dummy CPU action.
 *
 *
 * 2.
 * Note that bound == 0 means no bound (i.e., unlimited). But, if a host has
 * multiple CPU cores, the CPU share of a computation task (or a VM) never
 * exceeds the capacity of a CPU core.
 */
void MSG_vm_set_bound(msg_vm_t vm, double bound)
{
	return simcall_vm_set_bound(vm, bound);
}


/** @brief Set the CPU affinity of a given VM.
 *  @ingroup msg_VMs
 *
 * This function changes the CPU affinity of a given VM. Usage is the same as
 * MSG_task_set_affinity(). See the MSG_task_set_affinity() for details.
 */
void MSG_vm_set_affinity(msg_vm_t vm, msg_host_t pm, unsigned long mask)
{
  msg_host_priv_t priv = msg_host_resource_priv(vm);

  if (mask == 0)
    xbt_dict_remove_ext(priv->affinity_mask_db, (char *) pm, sizeof(pm));
  else
    xbt_dict_set_ext(priv->affinity_mask_db, (char *) pm, sizeof(pm), (void *) mask, NULL);

  msg_host_t pm_now = MSG_vm_get_pm(vm);
  if (pm_now == pm) {
    XBT_DEBUG("set affinity(0x%04lx@%s) for %s", mask, MSG_host_get_name(pm), MSG_host_get_name(vm));
    simcall_vm_set_affinity(vm, pm, mask);
  } else
    XBT_DEBUG("set affinity(0x%04lx@%s) for %s (not active now)", mask, MSG_host_get_name(pm), MSG_host_get_name(vm));
}
