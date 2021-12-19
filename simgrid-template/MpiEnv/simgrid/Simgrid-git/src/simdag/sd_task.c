/* Copyright (c) 2006-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "private.h"
#include "simdag/simdag.h"
#include "xbt/sysdep.h"
#include "xbt/dynar.h"
#include "instr/instr_private.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(sd_task, sd,
                                "Logging specific to SimDag (task)");

static void __SD_task_remove_dependencies(SD_task_t task);
static void __SD_task_destroy_scheduling_data(SD_task_t task);

void* SD_task_new_f(void)
{
  SD_task_t task = xbt_new0(s_SD_task_t, 1);
  task->tasks_before = xbt_dynar_new(sizeof(SD_dependency_t), NULL);
  task->tasks_after = xbt_dynar_new(sizeof(SD_dependency_t), NULL);

  return task;
}

void SD_task_recycle_f(void *t)
{
  SD_task_t task = (SD_task_t) t;

  /* Reset the content */
  task->kind = SD_TASK_NOT_TYPED;
  task->state_hookup.prev = NULL;
  task->state_hookup.next = NULL;
  task->state_set = sd_global->not_scheduled_task_set;
  xbt_swag_insert(task, task->state_set);
  task->state = SD_NOT_SCHEDULED;
  task->return_hookup.prev = NULL;
  task->return_hookup.next = NULL;

  task->marked = 0;

  task->start_time = -1.0;
  task->finish_time = -1.0;
  task->surf_action = NULL;
  task->watch_points = 0;

  /* dependencies */
  xbt_dynar_reset(task->tasks_before);
  xbt_dynar_reset(task->tasks_after);
  task->unsatisfied_dependencies = 0;
  task->is_not_ready = 0;

  /* scheduling parameters */
  task->workstation_nb = 0;
  task->workstation_list = NULL;
  task->computation_amount = NULL;
  task->communication_amount = NULL;
  task->rate = -1;
}

void SD_task_free_f(void *t)
{
  SD_task_t task = (SD_task_t)t;

  xbt_dynar_free(&task->tasks_before);
  xbt_dynar_free(&task->tasks_after);
  xbt_free(task);
}

/**
 * \brief Creates a new task.
 *
 * \param name the name of the task (can be \c NULL)
 * \param data the user data you want to associate with the task (can be \c NULL)
 * \param amount amount of the task
 * \return the new task
 * \see SD_task_destroy()
 */
SD_task_t SD_task_create(const char *name, void *data, double amount)
{
  SD_task_t task = xbt_mallocator_get(sd_global->task_mallocator);

  /* general information */
  task->data = data;            /* user data */
  task->name = xbt_strdup(name);
  task->amount = amount;
  task->remains = amount;

  sd_global->task_number++;

#ifdef HAVE_TRACING
  TRACE_sd_task_create(task);
#endif

  return task;
}

static XBT_INLINE SD_task_t SD_task_create_sized(const char *name,
                                                 void *data, double amount,
                                                 int ws_count)
{
  SD_task_t task = SD_task_create(name, data, amount);
  task->communication_amount = xbt_new0(double, ws_count * ws_count);
  task->computation_amount = xbt_new0(double, ws_count);
  task->workstation_nb = ws_count;
  task->workstation_list = xbt_new0(SD_workstation_t, ws_count);
  return task;
}

/** @brief create a end-to-end communication task that can then be auto-scheduled
 *
 * Auto-scheduling mean that the task can be used with SD_task_schedulev(). This
 * allows to specify the task costs at creation, and decouple them from the
 * scheduling process where you just specify which resource should deliver the
 * mandatory power.
 *
 * A end-to-end communication must be scheduled on 2 hosts, and the amount
 * specified at creation is sent from hosts[0] to hosts[1].
 */
SD_task_t SD_task_create_comm_e2e(const char *name, void *data,
                                  double amount)
{
  SD_task_t res = SD_task_create_sized(name, data, amount, 2);
  res->communication_amount[2] = amount;
  res->kind = SD_TASK_COMM_E2E;

#ifdef HAVE_TRACING
  TRACE_category("COMM_E2E");
  TRACE_sd_set_task_category(res, "COMM_E2E");
#endif

  return res;
}

/** @brief create a sequential computation task that can then be auto-scheduled
 *
 * Auto-scheduling mean that the task can be used with SD_task_schedulev(). This
 * allows to specify the task costs at creation, and decouple them from the
 * scheduling process where you just specify which resource should deliver the
 * mandatory power.
 *
 * A sequential computation must be scheduled on 1 host, and the amount
 * specified at creation to be run on hosts[0].
 *
 * \param name the name of the task (can be \c NULL)
 * \param data the user data you want to associate with the task (can be \c NULL)
 * \param amount amount of compute work to be done by the task
 * \return the new SD_TASK_COMP_SEQ typed task
 */
SD_task_t SD_task_create_comp_seq(const char *name, void *data,
                                  double amount)
{
  SD_task_t res = SD_task_create_sized(name, data, amount, 1);
  res->computation_amount[0] = amount;
  res->kind = SD_TASK_COMP_SEQ;

#ifdef HAVE_TRACING
  TRACE_category("COMP_SEQ");
  TRACE_sd_set_task_category(res, "COMP_SEQ");
#endif

return res;
}

/** @brief create a parallel computation task that can then be auto-scheduled
 *
 * Auto-scheduling mean that the task can be used with SD_task_schedulev(). This
 * allows to specify the task costs at creation, and decouple them from the
 * scheduling process where you just specify which resource should deliver the
 * mandatory power.
 *
 * A parallel computation can be scheduled on any number of host.
 * The underlying speedup model is Amdahl's law.
 * To be auto-scheduled, \see SD_task_distribute_comp_amdahl has to be called
 * first.
 * \param name the name of the task (can be \c NULL)
 * \param data the user data you want to associate with the task (can be \c NULL)
 * \param amount amount of compute work to be done by the task
 * \param alpha purely serial fraction of the work to be done (in [0.;1.[)
 * \return the new task
 */
SD_task_t SD_task_create_comp_par_amdahl(const char *name, void *data,
                                  double amount, double alpha)
{
  xbt_assert(alpha < 1. && alpha >= 0.,
              "Invalid parameter: alpha must be in [0.;1.[");
  
  SD_task_t res = SD_task_create(name, data, amount);
  res->alpha = alpha;
  res->kind = SD_TASK_COMP_PAR_AMDAHL;

#ifdef HAVE_TRACING
  TRACE_category("COMP_PAR_AMDAHL");
  TRACE_sd_set_task_category(res, "COMP_PAR_AMDAHL");
#endif

  return res;
}

/** @brief create a complex data redistribution task that can then be 
 * auto-scheduled
 *
 * Auto-scheduling mean that the task can be used with SD_task_schedulev(). 
 * This allows to specify the task costs at creation, and decouple them from 
 * the scheduling process where you just specify which resource should 
 * communicate. 
 *
 * A data redistribution can be scheduled on any number of host.
 * The assumed distribution is a 1D block distribution. Each host owns the same
 * share of the \see amount. 
 * To be auto-scheduled, \see SD_task_distribute_comm_mxn_1d_block has to be 
 * called first.
 * \param name the name of the task (can be \c NULL)
 * \param data the user data you want to associate with the task (can be
 * \c NULL)
 * \param amount amount of data to redistribute by the task
 * \return the new task
 */
SD_task_t SD_task_create_comm_par_mxn_1d_block(const char *name, void *data,
                                               double amount)
{
  SD_task_t res = SD_task_create(name, data, amount);
  res->workstation_list=NULL;
  res->kind = SD_TASK_COMM_PAR_MXN_1D_BLOCK;

#ifdef HAVE_TRACING
  TRACE_category("COMM_PAR_MXN_1D_BLOCK");
  TRACE_sd_set_task_category(res, "COMM_PAR_MXN_1D_BLOCK");
#endif

  return res;
}

/**
 * \brief Destroys a task.
 *
 * The user data (if any) should have been destroyed first.
 *
 * \param task the task you want to destroy
 * \see SD_task_create()
 */
void SD_task_destroy(SD_task_t task)
{
  XBT_DEBUG("Destroying task %s...", SD_task_get_name(task));

  __SD_task_remove_dependencies(task);
  /* if the task was scheduled or runnable we have to free the scheduling parameters */
  if (__SD_task_is_scheduled_or_runnable(task))
    __SD_task_destroy_scheduling_data(task);
  if (task->state_set != NULL) /* would be null if just created */
    xbt_swag_remove(task, task->state_set);

  xbt_swag_remove(task, sd_global->return_set);

  xbt_free(task->name);

  if (task->surf_action != NULL)
	surf_action_unref(task->surf_action);

  xbt_free(task->workstation_list);
  xbt_free(task->communication_amount);
  xbt_free(task->computation_amount);

#ifdef HAVE_TRACING
  TRACE_sd_task_destroy(task);
#endif

  xbt_mallocator_release(sd_global->task_mallocator,task);
  sd_global->task_number--;

  XBT_DEBUG("Task destroyed.");
}

/**
 * \brief Returns the user data of a task
 *
 * \param task a task
 * \return the user data associated with this task (can be \c NULL)
 * \see SD_task_set_data()
 */
void *SD_task_get_data(SD_task_t task)
{
  return task->data;
}

/**
 * \brief Sets the user data of a task
 *
 * The new data can be \c NULL. The old data should have been freed first
 * if it was not \c NULL.
 *
 * \param task a task
 * \param data the new data you want to associate with this task
 * \see SD_task_get_data()
 */
void SD_task_set_data(SD_task_t task, void *data)
{
  task->data = data;
}

/**
 * \brief Sets the rate of a task
 *
 * This will change the network bandwidth a task can use. This rate
 * depends on both the nominal bandwidth on the route onto which the task is
 * scheduled (\see SD_task_get_current_bandwidth) and the amount of data to
 * transfer.
 *
 * To divide the nominal bandwidth by 2, the rate then has to be :
 *    rate = bandwidth/(2*amount)
 *
 * \param task a \see SD_TASK_COMM_E2E task (end-to-end communication)
 * \param rate the new rate you want to associate with this task.
 */
void SD_task_set_rate(SD_task_t task, double rate)
{
  xbt_assert(task->kind == SD_TASK_COMM_E2E,
             "The rate can be modified for end-to-end communications only.");

  task->rate = rate;
}

/**
 * \brief Returns the state of a task
 *
 * \param task a task
 * \return the current \ref e_SD_task_state_t "state" of this task:
 * #SD_NOT_SCHEDULED, #SD_SCHEDULED, #SD_RUNNABLE, #SD_RUNNING, #SD_DONE or #SD_FAILED
 * \see e_SD_task_state_t
 */
e_SD_task_state_t SD_task_get_state(SD_task_t task)
{
  return task->state;
}

/* Changes the state of a task. Updates the swags and the flag sd_global->watch_point_reached.
 */
void __SD_task_set_state(SD_task_t task, e_SD_task_state_t new_state)
{
  xbt_swag_remove(task, task->state_set);
  switch (new_state) {
  case SD_NOT_SCHEDULED:
    task->state_set = sd_global->not_scheduled_task_set;
    break;
  case SD_SCHEDULABLE:
    task->state_set = sd_global->schedulable_task_set;
    break;
  case SD_SCHEDULED:
    task->state_set = sd_global->scheduled_task_set;
    break;
  case SD_RUNNABLE:
    task->state_set = sd_global->runnable_task_set;
    break;
  case SD_IN_FIFO:
    task->state_set = sd_global->in_fifo_task_set;
    break;
  case SD_RUNNING:
    task->state_set = sd_global->running_task_set;
    task->start_time = surf_action_get_start_time(task->surf_action);
    break;
  case SD_DONE:
    task->state_set = sd_global->done_task_set;
    task->finish_time = surf_action_get_finish_time(task->surf_action);
    task->remains = 0;
#ifdef HAVE_JEDULE
    jedule_log_sd_event(task);
#endif
    break;
  case SD_FAILED:
    task->state_set = sd_global->failed_task_set;
    break;
  default:
    xbt_die( "Invalid state");
  }
  xbt_swag_insert(task, task->state_set);
  task->state = new_state;

  if (task->watch_points & new_state) {
    XBT_VERB("Watch point reached with task '%s'!", SD_task_get_name(task));
    sd_global->watch_point_reached = 1;
    SD_task_unwatch(task, new_state);   /* remove the watch point */
  }
}

/**
 * \brief Returns the name of a task
 *
 * \param task a task
 * \return the name of this task (can be \c NULL)
 */
const char *SD_task_get_name(SD_task_t task)
{
  return task->name;
}

/** @brief Allows to change the name of a task */
void SD_task_set_name(SD_task_t task, const char *name)
{
  xbt_free(task->name);
  task->name = xbt_strdup(name);
}

/** @brief Returns the dynar of the parents of a task
 *
 * \param task a task
 * \return a newly allocated dynar comprising the parents of this task
 */

xbt_dynar_t SD_task_get_parents(SD_task_t task)
{
  unsigned int i;
  xbt_dynar_t parents;
  SD_dependency_t dep;

  parents = xbt_dynar_new(sizeof(SD_task_t), NULL);
  xbt_dynar_foreach(task->tasks_before, i, dep) {
    xbt_dynar_push(parents, &(dep->src));
  }
  return parents;
}

/** @brief Returns the dynar of the parents of a task
 *
 * \param task a task
 * \return a newly allocated dynar comprising the parents of this task
 */
xbt_dynar_t SD_task_get_children(SD_task_t task)
{
  unsigned int i;
  xbt_dynar_t children;
  SD_dependency_t dep;

  children = xbt_dynar_new(sizeof(SD_task_t), NULL);
  xbt_dynar_foreach(task->tasks_after, i, dep) {
    xbt_dynar_push(children, &(dep->dst));
  }
  return children;
}

/**
 * \brief Returns the amount of workstations involved in a task
 *
 * Only call this on already scheduled tasks!
 * \param task a task
 */
int SD_task_get_workstation_count(SD_task_t task)
{
  return task->workstation_nb;
}

/**
 * \brief Returns the list of workstations involved in a task
 *
 * Only call this on already scheduled tasks!
 * \param task a task
 */
SD_workstation_t *SD_task_get_workstation_list(SD_task_t task)
{
  return task->workstation_list;
}

/**
 * \brief Returns the total amount of work contained in a task
 *
 * \param task a task
 * \return the total amount of work (computation or data transfer) for this task
 * \see SD_task_get_remaining_amount()
 */
double SD_task_get_amount(SD_task_t task)
{
  return task->amount;
}

/**
 * \brief Sets the total amount of work of a task
 * For sequential typed tasks (COMP_SEQ and COMM_E2E), it also sets the
 * appropriate values in the computation_amount and communication_amount arrays
 * respectively. Nothing more than modifying task->amount is done for paralle
 * typed tasks (COMP_PAR_AMDAHL and COMM_PAR_MXN_1D_BLOCK) as the distribution
 * of the amount of work is done at scheduling time.
 *
 * \param task a task
 * \param amount the new amount of work to execute
 */
void SD_task_set_amount(SD_task_t task, double amount)
{
  task->amount = amount;
  if (task->kind == SD_TASK_COMP_SEQ)
    task->computation_amount[0] = amount;
  if (task->kind == SD_TASK_COMM_E2E)
    task->communication_amount[2] = amount;
}

/**
 * \brief Returns the alpha parameter of a SD_TASK_COMP_PAR_AMDAHL task
 *
 * \param task a parallel task assuming Amdahl's law as speedup model
 * \return the alpha parameter (serial part of a task in percent) for this task
 */
double SD_task_get_alpha(SD_task_t task)
{
  xbt_assert(SD_task_get_kind(task) == SD_TASK_COMP_PAR_AMDAHL,
     "Alpha parameter is not defined for this kink of task");
  return task->alpha;
}


/**
 * \brief Returns the remaining amount work to do till the completion of a task
 *
 * \param task a task
 * \return the remaining amount of work (computation or data transfer) of this task
 * \see SD_task_get_amount()
 */
double SD_task_get_remaining_amount(SD_task_t task)
{
  if (task->surf_action)
	return surf_action_get_remains(task->surf_action);
  else
    return task->remains;
}

int SD_task_get_kind(SD_task_t task)
{
  return task->kind;
}

/** @brief Displays debugging informations about a task */
void SD_task_dump(SD_task_t task)
{
  unsigned int counter;
  SD_dependency_t dependency;
  char *statename;

  XBT_INFO("Displaying task %s", SD_task_get_name(task));
  statename = bprintf("%s %s %s %s %s %s %s %s",
                      (task->state == SD_NOT_SCHEDULED ? "not scheduled" :
                       ""),
                      (task->state == SD_SCHEDULABLE ? "schedulable" : ""),
                      (task->state == SD_SCHEDULED ? "scheduled" : ""),
                      (task->state == SD_RUNNABLE ? "runnable" :
                       "not runnable"),
                      (task->state == SD_IN_FIFO ? "in fifo" : ""),
                      (task->state == SD_RUNNING ? "running" : ""),
                      (task->state == SD_DONE ? "done" : ""),
                      (task->state == SD_FAILED ? "failed" : ""));
  XBT_INFO("  - state: %s", statename);
  free(statename);

  if (task->kind != 0) {
    switch (task->kind) {
    case SD_TASK_COMM_E2E:
      XBT_INFO("  - kind: end-to-end communication");
      break;
    case SD_TASK_COMP_SEQ:
      XBT_INFO("  - kind: sequential computation");
      break;
    case SD_TASK_COMP_PAR_AMDAHL:
      XBT_INFO("  - kind: parallel computation following Amdahl's law");
      break;
    case SD_TASK_COMM_PAR_MXN_1D_BLOCK:
      XBT_INFO("  - kind: MxN data redistribution assuming 1D block distribution");
      break;
    default:
      XBT_INFO("  - (unknown kind %d)", task->kind);
    }
  }

#ifdef HAVE_TRACING
  if (task->category)
    XBT_INFO("  - tracing category: %s", task->category);
#endif

  XBT_INFO("  - amount: %.0f", SD_task_get_amount(task));
  if (task->kind == SD_TASK_COMP_PAR_AMDAHL)
    XBT_INFO("  - alpha: %.2f", task->alpha);
  XBT_INFO("  - Dependencies to satisfy: %d", task->unsatisfied_dependencies);
  if (!xbt_dynar_is_empty(task->tasks_before)) {
    XBT_INFO("  - pre-dependencies:");
    xbt_dynar_foreach(task->tasks_before, counter, dependency) {
      XBT_INFO("    %s", SD_task_get_name(dependency->src));
    }
  }
  if (!xbt_dynar_is_empty(task->tasks_after)) {
    XBT_INFO("  - post-dependencies:");
    xbt_dynar_foreach(task->tasks_after, counter, dependency) {
      XBT_INFO("    %s", SD_task_get_name(dependency->dst));
    }
  }
}

/** @brief Dumps the task in dotty formalism into the FILE* passed as second argument */
void SD_task_dotty(SD_task_t task, void *out)
{
  unsigned int counter;
  SD_dependency_t dependency;
  fprintf(out, "  T%p [label=\"%.20s\"", task, task->name);
  switch (task->kind) {
  case SD_TASK_COMM_E2E:
  case SD_TASK_COMM_PAR_MXN_1D_BLOCK:
    fprintf(out, ", shape=box");
    break;
  case SD_TASK_COMP_SEQ:
  case SD_TASK_COMP_PAR_AMDAHL:
    fprintf(out, ", shape=circle");
    break;
  default:
    xbt_die("Unknown task type!");
  }
  fprintf(out, "];\n");
  xbt_dynar_foreach(task->tasks_before, counter, dependency) {
    fprintf(out, " T%p -> T%p;\n", dependency->src, dependency->dst);
  }
}

/* Destroys a dependency between two tasks.
 */
static void __SD_task_dependency_destroy(void *dependency)
{
  xbt_free(((SD_dependency_t)dependency)->name);
  xbt_free(dependency);
}

/**
 * \brief Adds a dependency between two tasks
 *
 * \a dst will depend on \a src, ie \a dst will not start before \a src is finished.
 * Their \ref e_SD_task_state_t "state" must be #SD_NOT_SCHEDULED, #SD_SCHEDULED or #SD_RUNNABLE.
 *
 * \param name the name of the new dependency (can be \c NULL)
 * \param data the user data you want to associate with this dependency (can be \c NULL)
 * \param src the task which must be executed first
 * \param dst the task you want to make depend on \a src
 * \see SD_task_dependency_remove()
 */
void SD_task_dependency_add(const char *name, void *data, SD_task_t src,
                            SD_task_t dst)
{
  xbt_dynar_t dynar;
  unsigned long length;
  int found = 0;
  unsigned long i;
  SD_dependency_t dependency;

  dynar = src->tasks_after;
  length = xbt_dynar_length(dynar);

  if (src == dst)
    THROWF(arg_error, 0,
           "Cannot add a dependency between task '%s' and itself",
           SD_task_get_name(src));

  if (!__SD_task_is_not_scheduled(src) && !__SD_task_is_schedulable(src)
      && !__SD_task_is_scheduled_or_runnable(src) && !__SD_task_is_running(src))
    THROWF(arg_error, 0,
           "Task '%s' must be SD_NOT_SCHEDULED, SD_SCHEDULABLE, SD_SCHEDULED or SD_RUNNABLE"
     " or SD_RUNNING",
           SD_task_get_name(src));

  if (!__SD_task_is_not_scheduled(dst) && !__SD_task_is_schedulable(dst)
      && !__SD_task_is_scheduled_or_runnable(dst))
    THROWF(arg_error, 0,
           "Task '%s' must be SD_NOT_SCHEDULED, SD_SCHEDULABLE, SD_SCHEDULED or SD_RUNNABLE",
           SD_task_get_name(dst));

  XBT_DEBUG("SD_task_dependency_add: src = %s, dst = %s",
         SD_task_get_name(src), SD_task_get_name(dst));
  for (i = 0; i < length && !found; i++) {
    xbt_dynar_get_cpy(dynar, i, &dependency);
    found = (dependency->dst == dst);
    XBT_DEBUG("Dependency %lu: dependency->dst = %s", i,
           SD_task_get_name(dependency->dst));
  }

  if (found)
    THROWF(arg_error, 0,
           "A dependency already exists between task '%s' and task '%s'",
           SD_task_get_name(src), SD_task_get_name(dst));

  dependency = xbt_new(s_SD_dependency_t, 1);

  dependency->name = xbt_strdup(name);  /* xbt_strdup is cleaver enough to deal with NULL args itself */
  dependency->data = data;
  dependency->src = src;
  dependency->dst = dst;

  /* src must be executed before dst */
  xbt_dynar_push(src->tasks_after, &dependency);
  xbt_dynar_push(dst->tasks_before, &dependency);

  dst->unsatisfied_dependencies++;
  dst->is_not_ready++;

  /* if the task was runnable, then dst->tasks_before is not empty anymore,
     so we must go back to state SD_SCHEDULED */
  if (__SD_task_is_runnable(dst)) {
    XBT_DEBUG
        ("SD_task_dependency_add: %s was runnable and becomes scheduled!",
         SD_task_get_name(dst));
    __SD_task_set_state(dst, SD_SCHEDULED);
  }
}
/**
 * \brief Returns the name given as input when dependency has been created..
 *
 * \param src a task
 * \param dst a task depending on \a src
 *
 */
const char *SD_task_dependency_get_name(SD_task_t src, SD_task_t dst){
  unsigned int i;
  SD_dependency_t dependency;

  xbt_dynar_foreach(src->tasks_after, i, dependency){
    if (dependency->dst == dst)
      return dependency->name;
  }
  return NULL;
}

/**
 * \brief Indicates whether there is a dependency between two tasks.
 *
 * \param src a task
 * \param dst a task depending on \a src
 *
 * If src is NULL, checks whether dst has any pre-dependency.
 * If dst is NULL, checks whether src has any post-dependency.
 */
int SD_task_dependency_exists(SD_task_t src, SD_task_t dst)
{
  unsigned int counter;
  SD_dependency_t dependency;

  xbt_assert(src != NULL
              || dst != NULL,
              "Invalid parameter: both src and dst are NULL");

  if (src) {
    if (dst) {
      xbt_dynar_foreach(src->tasks_after, counter, dependency) {
        if (dependency->dst == dst)
          return 1;
      }
    } else {
      return xbt_dynar_length(src->tasks_after);
    }
  } else {
    return xbt_dynar_length(dst->tasks_before);
  }
  return 0;
}

/**
 * \brief Remove a dependency between two tasks
 *
 * \param src a task
 * \param dst a task depending on \a src
 * \see SD_task_dependency_add()
 */
void SD_task_dependency_remove(SD_task_t src, SD_task_t dst)
{

  xbt_dynar_t dynar;
  unsigned long length;
  int found = 0;
  unsigned long i;
  SD_dependency_t dependency;

  /* remove the dependency from src->tasks_after */
  dynar = src->tasks_after;
  length = xbt_dynar_length(dynar);

  for (i = 0; i < length && !found; i++) {
    xbt_dynar_get_cpy(dynar, i, &dependency);
    if (dependency->dst == dst) {
      xbt_dynar_remove_at(dynar, i, NULL);
      found = 1;
    }
  }
  if (!found)
    THROWF(arg_error, 0,
           "No dependency found between task '%s' and '%s': task '%s' is not a successor of task '%s'",
           SD_task_get_name(src), SD_task_get_name(dst),
           SD_task_get_name(dst), SD_task_get_name(src));

  /* remove the dependency from dst->tasks_before */
  dynar = dst->tasks_before;
  length = xbt_dynar_length(dynar);
  found = 0;

  for (i = 0; i < length && !found; i++) {
    xbt_dynar_get_cpy(dynar, i, &dependency);
    if (dependency->src == src) {
      xbt_dynar_remove_at(dynar, i, NULL);
      __SD_task_dependency_destroy(dependency);
      dst->unsatisfied_dependencies--;
      dst->is_not_ready--;
      found = 1;
    }
  }
  /* should never happen... */
  xbt_assert(found,
              "SimDag error: task '%s' is a successor of '%s' but task '%s' is not a predecessor of task '%s'",
              SD_task_get_name(dst), SD_task_get_name(src),
              SD_task_get_name(src), SD_task_get_name(dst));

  /* if the task was scheduled and dst->tasks_before is empty now, we can make it runnable */

  if (dst->unsatisfied_dependencies == 0) {
    if (__SD_task_is_scheduled(dst))
      __SD_task_set_state(dst, SD_RUNNABLE);
    else
      __SD_task_set_state(dst, SD_SCHEDULABLE);
  }

  if (dst->is_not_ready == 0)
    __SD_task_set_state(dst, SD_SCHEDULABLE);

  /*  __SD_print_dependencies(src);
     __SD_print_dependencies(dst); */
}

/**
 * \brief Returns the user data associated with a dependency between two tasks
 *
 * \param src a task
 * \param dst a task depending on \a src
 * \return the user data associated with this dependency (can be \c NULL)
 * \see SD_task_dependency_add()
 */
void *SD_task_dependency_get_data(SD_task_t src, SD_task_t dst)
{

  xbt_dynar_t dynar;
  unsigned long length;
  int found = 0;
  unsigned long i;
  SD_dependency_t dependency;

  dynar = src->tasks_after;
  length = xbt_dynar_length(dynar);

  for (i = 0; i < length && !found; i++) {
    xbt_dynar_get_cpy(dynar, i, &dependency);
    found = (dependency->dst == dst);
  }
  if (!found)
    THROWF(arg_error, 0, "No dependency found between task '%s' and '%s'",
           SD_task_get_name(src), SD_task_get_name(dst));
  return dependency->data;
}

/* temporary function for debugging */
static void __SD_print_watch_points(SD_task_t task)
{
  static const int state_masks[] =
      { SD_SCHEDULABLE, SD_SCHEDULED, SD_RUNNING, SD_RUNNABLE, SD_DONE,
    SD_FAILED
  };
  static const char *state_names[] =
      { "schedulable", "scheduled", "running", "runnable", "done",
    "failed"
  };
  int i;

  XBT_INFO("Task '%s' watch points (%x): ", SD_task_get_name(task),
        task->watch_points);


  for (i = 0; i < 5; i++) {
    if (task->watch_points & state_masks[i])
      XBT_INFO("%s ", state_names[i]);
  }
}

/**
 * \brief Adds a watch point to a task
 *
 * SD_simulate() will stop as soon as the \ref e_SD_task_state_t "state" of this
 * task becomes the one given in argument. The
 * watch point is then automatically removed.
 *
 * \param task a task
 * \param state the \ref e_SD_task_state_t "state" you want to watch
 * (cannot be #SD_NOT_SCHEDULED)
 * \see SD_task_unwatch()
 */
void SD_task_watch(SD_task_t task, e_SD_task_state_t state)
{
  if (state & SD_NOT_SCHEDULED)
    THROWF(arg_error, 0,
           "Cannot add a watch point for state SD_NOT_SCHEDULED");

  task->watch_points = task->watch_points | state;
  /*  __SD_print_watch_points(task); */
}

/**
 * \brief Removes a watch point from a task
 *
 * \param task a task
 * \param state the \ref e_SD_task_state_t "state" you no longer want to watch
 * \see SD_task_watch()
 */
void SD_task_unwatch(SD_task_t task, e_SD_task_state_t state)
{
  xbt_assert(state != SD_NOT_SCHEDULED,
              "SimDag error: Cannot have a watch point for state SD_NOT_SCHEDULED");

  task->watch_points = task->watch_points & ~state;
  /*  __SD_print_watch_points(task); */
}

/**
 * \brief Returns an approximative estimation of the execution time of a task.
 *
 * The estimation is very approximative because the value returned is the time
 * the task would take if it was executed now and if it was the only task.
 *
 * \param task the task to evaluate
 * \param workstation_nb number of workstations on which the task would be executed
 * \param workstation_list the workstations on which the task would be executed
 * \param computation_amount computation amount for each workstation
 * \param communication_amount communication amount between each pair of workstations
 * \see SD_schedule()
 */
double SD_task_get_execution_time(SD_task_t task,
                                  int workstation_nb,
                                  const SD_workstation_t *
                                  workstation_list,
                                  const double *computation_amount,
                                  const double *communication_amount)
{
  double time, max_time = 0.0;
  int i, j;
  xbt_assert(workstation_nb > 0, "Invalid parameter");

  /* the task execution time is the maximum execution time of the parallel tasks */

  for (i = 0; i < workstation_nb; i++) {
    time = 0.0;
    if (computation_amount != NULL)
      time =
          SD_workstation_get_computation_time(workstation_list[i],
                                              computation_amount[i]);

    if (communication_amount != NULL)
      for (j = 0; j < workstation_nb; j++) {
        time +=
            SD_route_get_communication_time(workstation_list[i],
                                            workstation_list[j],
                                            communication_amount[i *
                                                                 workstation_nb
                                                                 + j]);
      }

    if (time > max_time) {
      max_time = time;
    }
  }
  return max_time;
}

static XBT_INLINE void SD_task_do_schedule(SD_task_t task)
{
  if (!__SD_task_is_not_scheduled(task) && !__SD_task_is_schedulable(task))
    THROWF(arg_error, 0, "Task '%s' has already been scheduled",
           SD_task_get_name(task));

  /* update the task state */
  if (task->unsatisfied_dependencies == 0)
    __SD_task_set_state(task, SD_RUNNABLE);
  else
    __SD_task_set_state(task, SD_SCHEDULED);
}

/**
 * \brief Schedules a task
 *
 * The task state must be #SD_NOT_SCHEDULED.
 * Once scheduled, a task will be executed as soon as possible in SD_simulate(),
 * i.e. when its dependencies are satisfied.
 *
 * \param task the task you want to schedule
 * \param workstation_count number of workstations on which the task will be executed
 * \param workstation_list the workstations on which the task will be executed
 * \param computation_amount computation amount for each workstation
 * \param communication_amount communication amount between each pair of workstations
 * \param rate task execution speed rate
 * \see SD_task_unschedule()
 */
void SD_task_schedule(SD_task_t task, int workstation_count,
                      const SD_workstation_t * workstation_list,
                      const double *computation_amount,
                      const double *communication_amount, double rate)
{
  int communication_nb;
  task->workstation_nb = 0;
  task->rate = -1;
  xbt_assert(workstation_count > 0, "workstation_nb must be positive");

  task->workstation_nb = workstation_count;
  task->rate = rate;

  if (computation_amount) {
    task->computation_amount = xbt_realloc(task->computation_amount,
                                           sizeof(double) * workstation_count);
    memcpy(task->computation_amount, computation_amount,
           sizeof(double) * workstation_count);
  } else {
    xbt_free(task->computation_amount);
    task->computation_amount = NULL;
  }

  communication_nb = workstation_count * workstation_count;
  if (communication_amount) {
    task->communication_amount = xbt_realloc(task->communication_amount,
                                             sizeof(double) * communication_nb);
    memcpy(task->communication_amount, communication_amount,
           sizeof(double) * communication_nb);
  } else {
    xbt_free(task->communication_amount);
    task->communication_amount = NULL;
  }

  task->workstation_list =
    xbt_realloc(task->workstation_list,
                sizeof(SD_workstation_t) * workstation_count);
  memcpy(task->workstation_list, workstation_list,
         sizeof(SD_workstation_t) * workstation_count);

  SD_task_do_schedule(task);
}

/**
 * \brief Unschedules a task
 *
 * The task state must be #SD_SCHEDULED, #SD_RUNNABLE, #SD_RUNNING or #SD_FAILED.
 * If you call this function, the task state becomes #SD_NOT_SCHEDULED.
 * Call SD_task_schedule() to schedule it again.
 *
 * \param task the task you want to unschedule
 * \see SD_task_schedule()
 */
void SD_task_unschedule(SD_task_t task)
{
  if (task->state_set != sd_global->scheduled_task_set &&
      task->state_set != sd_global->runnable_task_set &&
      task->state_set != sd_global->running_task_set &&
      task->state_set != sd_global->failed_task_set)
    THROWF(arg_error, 0,
           "Task %s: the state must be SD_SCHEDULED, SD_RUNNABLE, SD_RUNNING or SD_FAILED",
           SD_task_get_name(task));

  if (__SD_task_is_scheduled_or_runnable(task)  /* if the task is scheduled or runnable */
      && ((task->kind == SD_TASK_COMP_PAR_AMDAHL) ||
          (task->kind == SD_TASK_COMM_PAR_MXN_1D_BLOCK))) { /* Don't free scheduling data for typed tasks */
    __SD_task_destroy_scheduling_data(task);
    xbt_free(task->workstation_list);
    task->workstation_list=NULL;
    task->workstation_nb = 0;
  }

  if (__SD_task_is_running(task))       /* the task should become SD_FAILED */
	surf_action_cancel(task->surf_action);
  else {
    if (task->unsatisfied_dependencies == 0)
      __SD_task_set_state(task, SD_SCHEDULABLE);
    else
      __SD_task_set_state(task, SD_NOT_SCHEDULED);
  }
  task->remains = task->amount;
  task->start_time = -1.0;
}

/* Destroys the data memorized by SD_task_schedule.
 * Task state must be SD_SCHEDULED or SD_RUNNABLE.
 */
static void __SD_task_destroy_scheduling_data(SD_task_t task)
{
  if (!__SD_task_is_scheduled_or_runnable(task)
      && !__SD_task_is_in_fifo(task))
    THROWF(arg_error, 0,
           "Task '%s' must be SD_SCHEDULED, SD_RUNNABLE or SD_IN_FIFO",
           SD_task_get_name(task));

  xbt_free(task->computation_amount);
  xbt_free(task->communication_amount);
  task->computation_amount = task->communication_amount = NULL;
}

/* Runs a task. This function is directly called by __SD_task_try_to_run if
 * the task doesn't have to wait in FIFOs. Otherwise, it is called by
 * __SD_task_just_done when the task gets out of its FIFOs.
 */
void __SD_task_really_run(SD_task_t task)
{

  int i;
  void **surf_workstations;

  xbt_assert(__SD_task_is_runnable_or_in_fifo(task),
              "Task '%s' is not runnable or in a fifo! Task state: %d",
             SD_task_get_name(task), (int)SD_task_get_state(task));
  xbt_assert(task->workstation_list != NULL,
              "Task '%s': workstation_list is NULL!",
              SD_task_get_name(task));

  XBT_DEBUG("Really running task '%s'", SD_task_get_name(task));
  int workstation_nb = task->workstation_nb;

  /* set this task as current task for the workstations in sequential mode */
  for (i = 0; i < workstation_nb; i++) {
    if (SD_workstation_get_access_mode(task->workstation_list[i]) ==
        SD_WORKSTATION_SEQUENTIAL_ACCESS) {
      SD_workstation_priv(task->workstation_list[i])->current_task = task;
      xbt_assert(__SD_workstation_is_busy(task->workstation_list[i]),
                  "The workstation should be busy now");
    }
  }

  XBT_DEBUG("Task '%s' set as current task for its workstations",
         SD_task_get_name(task));

  /* start the task */

  /* we have to create a Surf workstation array instead of the SimDag
   * workstation array */
  surf_workstations = xbt_new(void *, workstation_nb);

  for (i = 0; i < workstation_nb; i++)
    surf_workstations[i] =  surf_workstation_resource_priv(task->workstation_list[i]);

  double *computation_amount = xbt_new0(double, workstation_nb);
  double *communication_amount = xbt_new0(double, workstation_nb * workstation_nb);


  if(task->computation_amount)
    memcpy(computation_amount, task->computation_amount, sizeof(double) *
           workstation_nb);
  if(task->communication_amount)
    memcpy(communication_amount, task->communication_amount,
           sizeof(double) * workstation_nb * workstation_nb);

  task->surf_action = surf_workstation_model_execute_parallel_task((surf_workstation_model_t)surf_workstation_model,
		                                                     workstation_nb,
		                                                     surf_workstations,
		                                                     computation_amount,
		                                                     communication_amount,
		                                                     task->rate);

  surf_action_set_data(task->surf_action, task);

  XBT_DEBUG("surf_action = %p", task->surf_action);

#ifdef HAVE_TRACING
  if (task->category)
    TRACE_surf_action(task->surf_action, task->category);
#endif

  __SD_task_destroy_scheduling_data(task);      /* now the scheduling data are not useful anymore */
  __SD_task_set_state(task, SD_RUNNING);
  xbt_assert(__SD_task_is_running(task), "Bad state of task '%s': %d",
             SD_task_get_name(task), (int)SD_task_get_state(task));

}

/* Tries to run a task. This function is called by SD_simulate() when a
 * scheduled task becomes SD_RUNNABLE (i.e., when its dependencies are
 * satisfied).
 * If one of the workstations where the task is scheduled on is busy (in
 * sequential mode), the task doesn't start.
 * Returns whether the task has started.
 */
int __SD_task_try_to_run(SD_task_t task)
{

  int can_start = 1;
  int i;
  SD_workstation_t workstation;

  xbt_assert(__SD_task_is_runnable(task),
              "Task '%s' is not runnable! Task state: %d",
             SD_task_get_name(task), (int)SD_task_get_state(task));


  for (i = 0; i < task->workstation_nb; i++) {
    can_start = can_start &&
        !__SD_workstation_is_busy(task->workstation_list[i]);
  }

  XBT_DEBUG("Task '%s' can start: %d", SD_task_get_name(task), can_start);

  if (!can_start) {             /* if the task cannot start and is not in the FIFOs yet */
    for (i = 0; i < task->workstation_nb; i++) {
      workstation = task->workstation_list[i];
      if (SD_workstation_priv(workstation)->access_mode == SD_WORKSTATION_SEQUENTIAL_ACCESS) {
        XBT_DEBUG("Pushing task '%s' in the FIFO of workstation '%s'",
               SD_task_get_name(task),
               SD_workstation_get_name(workstation));
        xbt_fifo_push(SD_workstation_priv(workstation)->task_fifo, task);
      }
    }
    __SD_task_set_state(task, SD_IN_FIFO);
    xbt_assert(__SD_task_is_in_fifo(task), "Bad state of task '%s': %d",
               SD_task_get_name(task), (int)SD_task_get_state(task));
    XBT_DEBUG("Task '%s' state is now SD_IN_FIFO", SD_task_get_name(task));
  } else {
    __SD_task_really_run(task);
  }

  return can_start;
}

/* This function is called by SD_simulate when a task is done.
 * It updates task->state and task->action and executes if necessary the tasks
 * which were waiting in FIFOs for the end of `task'
 */
void __SD_task_just_done(SD_task_t task)
{
  int i, j;
  SD_workstation_t workstation;

  SD_task_t candidate;
  int candidate_nb = 0;
  int candidate_capacity = 8;
  SD_task_t *candidates;
  int can_start = 1;

  xbt_assert(__SD_task_is_running(task),
              "The task must be running! Task state: %d",
              (int)SD_task_get_state(task));
  xbt_assert(task->workstation_list != NULL,
              "Task '%s': workstation_list is NULL!",
              SD_task_get_name(task));


  candidates = xbt_new(SD_task_t, 8);

  __SD_task_set_state(task, SD_DONE);
  surf_action_unref(task->surf_action);
  task->surf_action = NULL;

  XBT_DEBUG("Looking for candidates");

  /* if the task was executed on sequential workstations,
     maybe we can execute the next task of the FIFO for each workstation */
  for (i = 0; i < task->workstation_nb; i++) {
    workstation = task->workstation_list[i];
    XBT_DEBUG("Workstation '%s': access_mode = %d",
              SD_workstation_get_name(workstation), (int)SD_workstation_priv(workstation)->access_mode);
    if (SD_workstation_priv(workstation)->access_mode == SD_WORKSTATION_SEQUENTIAL_ACCESS) {
      xbt_assert(SD_workstation_priv(workstation)->task_fifo != NULL,
                  "Workstation '%s' has sequential access but no FIFO!",
                  SD_workstation_get_name(workstation));
      xbt_assert(SD_workstation_priv(workstation)->current_task =
                  task, "Workstation '%s': current task should be '%s'",
                  SD_workstation_get_name(workstation),
                  SD_task_get_name(task));

      /* the task is over so we can release the workstation */
      SD_workstation_priv(workstation)->current_task = NULL;

      XBT_DEBUG("Getting candidate in FIFO");
      candidate =
          xbt_fifo_get_item_content(xbt_fifo_get_first_item
                                    (SD_workstation_priv(workstation)->task_fifo));

      if (candidate != NULL) {
        XBT_DEBUG("Candidate: '%s'", SD_task_get_name(candidate));
        xbt_assert(__SD_task_is_in_fifo(candidate),
                    "Bad state of candidate '%s': %d",
                    SD_task_get_name(candidate),
                    (int)SD_task_get_state(candidate));
      }

      XBT_DEBUG("Candidate in fifo: %p", candidate);

      /* if there was a task waiting for my place */
      if (candidate != NULL) {
        /* Unfortunately, we are not sure yet that we can execute the task now,
           because the task can be waiting more deeply in some other
           workstation's FIFOs ...
           So we memorize all candidate tasks, and then we will check for each
           candidate whether or not all its workstations are available. */

        /* realloc if necessary */
        if (candidate_nb == candidate_capacity) {
          candidate_capacity *= 2;
          candidates =
              xbt_realloc(candidates,
                          sizeof(SD_task_t) * candidate_capacity);
        }

        /* register the candidate */
        candidates[candidate_nb++] = candidate;
        candidate->fifo_checked = 0;
      }
    }
  }

  XBT_DEBUG("Candidates found: %d", candidate_nb);

  /* now we check every candidate task */
  for (i = 0; i < candidate_nb; i++) {
    candidate = candidates[i];

    if (candidate->fifo_checked) {
      continue;                 /* we have already evaluated that task */
    }

    xbt_assert(__SD_task_is_in_fifo(candidate),
                "Bad state of candidate '%s': %d",
               SD_task_get_name(candidate), (int)SD_task_get_state(candidate));

    for (j = 0; j < candidate->workstation_nb && can_start; j++) {
      workstation = candidate->workstation_list[j];

      /* I can start on this workstation if the workstation is shared
         or if I am the first task in the FIFO */
      can_start = SD_workstation_priv(workstation)->access_mode == SD_WORKSTATION_SHARED_ACCESS
          || candidate ==
          xbt_fifo_get_item_content(xbt_fifo_get_first_item
                                    (SD_workstation_priv(workstation)->task_fifo));
    }

    XBT_DEBUG("Candidate '%s' can start: %d", SD_task_get_name(candidate),
           can_start);

    /* now we are sure that I can start! */
    if (can_start) {
      for (j = 0; j < candidate->workstation_nb && can_start; j++) {
        workstation = candidate->workstation_list[j];

        /* update the FIFO */
        if (SD_workstation_priv(workstation)->access_mode == SD_WORKSTATION_SEQUENTIAL_ACCESS) {
          candidate = xbt_fifo_shift(SD_workstation_priv(workstation)->task_fifo);   /* the return value is stored just for debugging */
          XBT_DEBUG("Head of the FIFO: '%s'",
                 (candidate !=
                  NULL) ? SD_task_get_name(candidate) : "NULL");
          xbt_assert(candidate == candidates[i],
                      "Error in __SD_task_just_done: bad first task in the FIFO");
        }
      }                         /* for each workstation */

      /* finally execute the task */
      XBT_DEBUG("Task '%s' state: %d", SD_task_get_name(candidate),
             (int)SD_task_get_state(candidate));
      __SD_task_really_run(candidate);

      XBT_DEBUG
          ("Calling __SD_task_is_running: task '%s', state set: %p, running_task_set: %p, is running: %d",
           SD_task_get_name(candidate), candidate->state_set,
           sd_global->running_task_set, __SD_task_is_running(candidate));
      xbt_assert(__SD_task_is_running(candidate),
                  "Bad state of task '%s': %d",
                  SD_task_get_name(candidate),
                 (int)SD_task_get_state(candidate));
      XBT_DEBUG("Okay, the task is running.");

    }                           /* can start */
    candidate->fifo_checked = 1;
  }                             /* for each candidate */

  xbt_free(candidates);
}

/* 
 * Remove all dependencies associated with a task. This function is called 
 * when the task is destroyed.
 */
static void __SD_task_remove_dependencies(SD_task_t task)
{
  /* we must destroy the dependencies carefuly (with SD_dependency_remove)
     because each one is stored twice */
  SD_dependency_t dependency;
  while (!xbt_dynar_is_empty(task->tasks_before)) {
    xbt_dynar_get_cpy(task->tasks_before, 0, &dependency);
    SD_task_dependency_remove(dependency->src, dependency->dst);
  }

  while (!xbt_dynar_is_empty(task->tasks_after)) {
    xbt_dynar_get_cpy(task->tasks_after, 0, &dependency);
    SD_task_dependency_remove(dependency->src, dependency->dst);
  }
}

/**
 * \brief Returns the start time of a task
 *
 * The task state must be SD_RUNNING, SD_DONE or SD_FAILED.
 *
 * \param task: a task
 * \return the start time of this task
 */
double SD_task_get_start_time(SD_task_t task)
{
  if (task->surf_action)
    return surf_action_get_start_time(task->surf_action);
  else
    return task->start_time;
}

/**
 * \brief Returns the finish time of a task
 *
 * The task state must be SD_RUNNING, SD_DONE or SD_FAILED.
 * If the state is not completed yet, the returned value is an
 * estimation of the task finish time. This value can fluctuate
 * until the task is completed.
 *
 * \param task: a task
 * \return the start time of this task
 */
double SD_task_get_finish_time(SD_task_t task)
{
  if (task->surf_action)        /* should never happen as actions are destroyed right after their completion */
    return surf_action_get_finish_time(task->surf_action);
  else
    return task->finish_time;
}
/** @brief Blah
 *
 */
void SD_task_distribute_comp_amdahl(SD_task_t task, int ws_count)
{
  int i;
  xbt_assert(task->kind == SD_TASK_COMP_PAR_AMDAHL,
              "Task %s is not a SD_TASK_COMP_PAR_AMDAHL typed task."
              "Cannot use this function.",
              SD_task_get_name(task));  
  task->computation_amount = xbt_new0(double, ws_count);
  task->communication_amount = xbt_new0(double, ws_count * ws_count);
  xbt_free(task->workstation_list);
  task->workstation_nb = ws_count;
  task->workstation_list = xbt_new0(SD_workstation_t, ws_count);
  
  for(i=0;i<ws_count;i++){
    task->computation_amount[i] = 
      (task->alpha + (1 - task->alpha)/ws_count) * task->amount;
  }
} 


/** @brief Auto-schedules a task.
 *
 * Auto-scheduling mean that the task can be used with SD_task_schedulev(). This
 * allows to specify the task costs at creation, and decouple them from the
 * scheduling process where you just specify which resource should deliver the
 * mandatory power.
 *
 * To be auto-schedulable, a task must be created with SD_task_create_comm_e2e()
 * or SD_task_create_comp_seq(). Check their definitions for the exact semantic
 * of each of them.
 *
 * @todo
 * We should create tasks kind for the following categories:
 *  - Point to point communication (done)
 *  - Sequential computation       (done)
 *  - group communication (redistribution, several kinds)
 *  - parallel tasks with no internal communication (one kind per speedup
 *    model such as Amdahl)
 *  - idem+ internal communication. Task type not enough since we cannot store
 *    comm cost alongside to comp one)
 */
void SD_task_schedulev(SD_task_t task, int count,
                       const SD_workstation_t * list)
{
  int i, j;
  SD_dependency_t dep;
  unsigned int cpt;
  xbt_assert(task->kind != 0,
              "Task %s is not typed. Cannot automatically schedule it.",
              SD_task_get_name(task));
  switch (task->kind) {
  case SD_TASK_COMP_PAR_AMDAHL:
    SD_task_distribute_comp_amdahl(task, count);
  case SD_TASK_COMM_E2E:
  case SD_TASK_COMP_SEQ:
    xbt_assert(task->workstation_nb == count,
               "Got %d locations, but were expecting %d locations",
               count,task->workstation_nb);
    for (i = 0; i < count; i++)
      task->workstation_list[i] = list[i];
    if (SD_task_get_kind(task)== SD_TASK_COMP_SEQ && !task->computation_amount){
      /*This task has failed and is rescheduled. Reset the computation amount*/
      task->computation_amount = xbt_new0(double, 1);
      task->computation_amount[0] = task->remains;
    }
    SD_task_do_schedule(task);
    break;
  default:
    xbt_die("Kind of task %s not supported by SD_task_schedulev()",
            SD_task_get_name(task));
  }
  if (task->kind == SD_TASK_COMM_E2E) {
    XBT_VERB("Schedule comm task %s between %s -> %s. It costs %.f bytes",
          SD_task_get_name(task),
          SD_workstation_get_name(task->workstation_list[0]),
          SD_workstation_get_name(task->workstation_list[1]),
          task->communication_amount[2]);

  }

  /* Iterate over all children and parents being COMM_E2E to say where I am
   * located (and start them if runnable) */
  if (task->kind == SD_TASK_COMP_SEQ) {
    XBT_VERB("Schedule computation task %s on %s. It costs %.f flops",
          SD_task_get_name(task),
          SD_workstation_get_name(task->workstation_list[0]),
          task->computation_amount[0]);

    xbt_dynar_foreach(task->tasks_before, cpt, dep) {
      SD_task_t before = dep->src;
      if (before->kind == SD_TASK_COMM_E2E) {
        before->workstation_list[1] = task->workstation_list[0];

        if (before->workstation_list[0] &&
            (__SD_task_is_schedulable(before)
             || __SD_task_is_not_scheduled(before))) {
          SD_task_do_schedule(before);
          XBT_VERB
              ("Auto-Schedule comm task %s between %s -> %s. It costs %.f bytes",
               SD_task_get_name(before),
               SD_workstation_get_name(before->workstation_list[0]),
               SD_workstation_get_name(before->workstation_list[1]),
               before->communication_amount[2]);
        }
      }
    }
    xbt_dynar_foreach(task->tasks_after, cpt, dep) {
      SD_task_t after = dep->dst;
      if (after->kind == SD_TASK_COMM_E2E) {
        after->workstation_list[0] = task->workstation_list[0];
        if (after->workstation_list[1]
            && (__SD_task_is_not_scheduled(after)
                || __SD_task_is_schedulable(after))) {
          SD_task_do_schedule(after);
          XBT_VERB
              ("Auto-Schedule comm task %s between %s -> %s. It costs %.f bytes",
               SD_task_get_name(after),
               SD_workstation_get_name(after->workstation_list[0]),
               SD_workstation_get_name(after->workstation_list[1]),
               after->communication_amount[2]);

        }
      }
    }
  }
  /* Iterate over all children and parents being MXN_1D_BLOCK to say where I am
   * located (and start them if runnable) */
  if (task->kind == SD_TASK_COMP_PAR_AMDAHL) {
    XBT_VERB("Schedule computation task %s on %d workstations. %.f flops"
             " will be distributed following Amdahl's Law",
          SD_task_get_name(task), task->workstation_nb,
          task->computation_amount[0]);
    xbt_dynar_foreach(task->tasks_before, cpt, dep) {
      SD_task_t before = dep->src;
      if (before->kind == SD_TASK_COMM_PAR_MXN_1D_BLOCK){
        if (!before->workstation_list){
          XBT_VERB("Sender side of Task %s is not scheduled yet",
             SD_task_get_name(before));
          before->workstation_list = xbt_new0(SD_workstation_t, count);
          before->workstation_nb = count;
          XBT_VERB("Fill the workstation list with list of Task '%s'",
            SD_task_get_name(task));
          for (i=0;i<count;i++)
            before->workstation_list[i] = task->workstation_list[i];
        } else {
          XBT_VERB("Build communication matrix for task '%s'",
             SD_task_get_name(before));
          int src_nb, dst_nb;
          double src_start, src_end, dst_start, dst_end;
          src_nb = before->workstation_nb;
          dst_nb = count;
          before->workstation_list = (SD_workstation_t*) xbt_realloc(
             before->workstation_list,
             (before->workstation_nb+count)*sizeof(s_SD_workstation_t));
          for(i=0; i<count; i++)
            before->workstation_list[before->workstation_nb+i] =
               task->workstation_list[i];

          before->workstation_nb += count;
          xbt_free(before->computation_amount);
          xbt_free(before->communication_amount);
          before->computation_amount = xbt_new0(double,
                                                before->workstation_nb);
          before->communication_amount = xbt_new0(double,
                                                  before->workstation_nb*
                                                  before->workstation_nb);

          for(i=0;i<src_nb;i++){
            src_start = i*before->amount/src_nb;
            src_end = src_start + before->amount/src_nb;
            for(j=0; j<dst_nb; j++){
              dst_start = j*before->amount/dst_nb;
              dst_end = dst_start + before->amount/dst_nb;
              XBT_VERB("(%s->%s): (%.2f, %.2f)-> (%.2f, %.2f)",
                  SD_workstation_get_name(before->workstation_list[i]),
                  SD_workstation_get_name(before->workstation_list[src_nb+j]),
                  src_start, src_end, dst_start, dst_end);
              if ((src_end <= dst_start) || (dst_end <= src_start)) {
                before->communication_amount[i*(src_nb+dst_nb)+src_nb+j]=0.0;
              } else {
                before->communication_amount[i*(src_nb+dst_nb)+src_nb+j] =
                  MIN(src_end, dst_end) - MAX(src_start, dst_start);
              }
              XBT_VERB("==> %.2f",
                 before->communication_amount[i*(src_nb+dst_nb)+src_nb+j]);
            }
          }

          if (__SD_task_is_schedulable(before) ||
              __SD_task_is_not_scheduled(before)) {
            SD_task_do_schedule(before);
            XBT_VERB
              ("Auto-Schedule redistribution task %s. Send %.f bytes from %d hosts to %d hosts.",
                  SD_task_get_name(before),before->amount, src_nb, dst_nb);
            }
        }
      }
    }
    xbt_dynar_foreach(task->tasks_after, cpt, dep) {
      SD_task_t after = dep->dst;
      if (after->kind == SD_TASK_COMM_PAR_MXN_1D_BLOCK){
        if (!after->workstation_list){
          XBT_VERB("Receiver side of Task '%s' is not scheduled yet",
              SD_task_get_name(after));
          after->workstation_list = xbt_new0(SD_workstation_t, count);
          after->workstation_nb = count;
          XBT_VERB("Fill the workstation list with list of Task '%s'",
            SD_task_get_name(task));
          for (i=0;i<count;i++)
            after->workstation_list[i] = task->workstation_list[i];
        } else {
          int src_nb, dst_nb;
          double src_start, src_end, dst_start, dst_end;
          src_nb = count;
          dst_nb = after->workstation_nb;
          after->workstation_list = (SD_workstation_t*) xbt_realloc(
            after->workstation_list,
            (after->workstation_nb+count)*sizeof(s_SD_workstation_t));
          for(i=after->workstation_nb - 1; i>=0; i--)
            after->workstation_list[count+i] = after->workstation_list[i];
          for(i=0; i<count; i++)
            after->workstation_list[i] = task->workstation_list[i];

          after->workstation_nb += count;

          xbt_free(after->computation_amount);
          xbt_free(after->communication_amount);

          after->computation_amount = xbt_new0(double, after->workstation_nb);
          after->communication_amount = xbt_new0(double,
                                                 after->workstation_nb*
                                                 after->workstation_nb);

          for(i=0;i<src_nb;i++){
            src_start = i*after->amount/src_nb;
            src_end = src_start + after->amount/src_nb;
            for(j=0; j<dst_nb; j++){
              dst_start = j*after->amount/dst_nb;
              dst_end = dst_start + after->amount/dst_nb;
              XBT_VERB("(%d->%d): (%.2f, %.2f)-> (%.2f, %.2f)",
                  i, j, src_start, src_end, dst_start, dst_end);
              if ((src_end <= dst_start) || (dst_end <= src_start)) {
                after->communication_amount[i*(src_nb+dst_nb)+src_nb+j]=0.0;
              } else {
                after->communication_amount[i*(src_nb+dst_nb)+src_nb+j] =
                   MIN(src_end, dst_end)- MAX(src_start, dst_start);
              }
              XBT_VERB("==> %.2f",
                 after->communication_amount[i*(src_nb+dst_nb)+src_nb+j]);
            }
          }

          if (__SD_task_is_schedulable(after) ||
              __SD_task_is_not_scheduled(after)) {
            SD_task_do_schedule(after);
            XBT_VERB
            ("Auto-Schedule redistribution task %s. Send %.f bytes from %d hosts to %d hosts.",
              SD_task_get_name(after),after->amount, src_nb, dst_nb);
          }
         }
      }
    }
  }
}

/** @brief autoschedule a task on a list of workstations
 *
 * This function is very similar to SD_task_schedulev(),
 * but takes the list of workstations to schedule onto as separate parameters.
 * It builds a proper vector of workstations and then call SD_task_schedulev()
 */
void SD_task_schedulel(SD_task_t task, int count, ...)
{
  va_list ap;
  SD_workstation_t *list = xbt_new(SD_workstation_t, count);
  int i;
  va_start(ap, count);
  for (i = 0; i < count; i++) {
    list[i] = va_arg(ap, SD_workstation_t);
  }
  va_end(ap);
  SD_task_schedulev(task, count, list);
  free(list);
}
