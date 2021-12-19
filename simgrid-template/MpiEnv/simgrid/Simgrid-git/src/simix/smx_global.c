/* Copyright (c) 2007-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "smx_private.h"
#include "xbt/heap.h"
#include "xbt/sysdep.h"
#include "xbt/log.h"
#include "xbt/str.h"
#include "xbt/ex.h"             /* ex_backtrace_display */
#include "mc/mc.h"
#include "simgrid/sg_config.h"

#ifdef HAVE_MC
#include "mc/mc_private.h"
#endif

#ifdef HAVE_SMPI
#include "smpi/private.h"
#endif

XBT_LOG_NEW_CATEGORY(simix, "All SIMIX categories");
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(simix_kernel, simix,
                                "Logging specific to SIMIX (kernel)");

smx_global_t simix_global = NULL;
static xbt_heap_t simix_timers = NULL;

static void* SIMIX_synchro_mallocator_new_f(void);
static void SIMIX_synchro_mallocator_free_f(void* synchro);
static void SIMIX_synchro_mallocator_reset_f(void* synchro);

/* FIXME: Yeah, I'll do it in a portable maner one day [Mt] */
#include <signal.h>

int _sg_do_verbose_exit = 1;
static void _XBT_CALL inthandler(int ignored)
{
  if ( _sg_do_verbose_exit ) {
     XBT_INFO("CTRL-C pressed. The current status will be displayed before exit (disable that behavior with option 'verbose-exit').");
     SIMIX_display_process_status();
  }
  else {
     XBT_INFO("CTRL-C pressed, exiting. Hiding the current process status since 'verbose-exit' is set to false.");
  }
  exit(1);
}

#ifndef WIN32
static void _XBT_CALL segvhandler(int signum, siginfo_t *siginfo, void *context)
{
  if (siginfo->si_signo == SIGSEGV && siginfo->si_code == SEGV_ACCERR) {
    fprintf(stderr,
            "Access violation detected.\n"
            "This can result from a programming error in your code or, although less likely,\n"
            "from a bug in SimGrid itself.  This can also be the sign of a bug in the OS or\n"
            "in third-party libraries.  Failing hardware can sometimes generate such errors\n"
            "too.\n"
            "Finally, if nothing of the above applies, this can result from a stack overflow.\n"
            "Try to increase stack size with --cfg=contexts/stack_size (current size is %d KiB).\n",
            smx_context_stack_size / 1024);
    if (XBT_LOG_ISENABLED(simix_kernel, xbt_log_priority_debug)) {
      fprintf(stderr,
              "siginfo = {si_signo = %d, si_errno = %d, si_code = %d, si_addr = %p}\n",
              siginfo->si_signo, siginfo->si_errno, siginfo->si_code, siginfo->si_addr);
    }
  } else  if (siginfo->si_signo == SIGSEGV) {
    fprintf(stderr, "Segmentation fault.\n");
#ifdef HAVE_SMPI
    if (smpi_enabled() && !smpi_privatize_global_variables) {
      fprintf(stderr,
        "Try to enable SMPI variable privatization with --cfg=smpi/privatize_global_variables:yes.\n");
    }
#endif
  }
#ifdef HAVE_MC
  if (MC_is_active()) {
    if (mc_stack) {
      MC_dump_stack_safety(mc_stack);
    }
    MC_print_statistics(mc_stats);
  }
#endif
  raise(signum);
}

char sigsegv_stack[SIGSTKSZ];   /* alternate stack for SIGSEGV handler */

/**
 * Install signal handler for SIGSEGV.  Check that nobody has already installed
 * its own handler.  For example, the Java VM does this.
 */
static void install_segvhandler(void)
{
  stack_t stack, old_stack;
  stack.ss_sp = sigsegv_stack;
  stack.ss_size = sizeof sigsegv_stack;
  stack.ss_flags = 0;

  if (sigaltstack(&stack, &old_stack) == -1) {
    XBT_WARN("Failed to register alternate signal stack: %s",
             strerror(errno));
    return;
  }
  if (!(old_stack.ss_flags & SS_DISABLE)) {
    XBT_DEBUG("An alternate stack was already installed (sp=%p, size=%zd, flags=%x). Restore it.",
              old_stack.ss_sp, old_stack.ss_size, old_stack.ss_flags);
    sigaltstack(&old_stack, NULL);
  }

  struct sigaction action, old_action;
  action.sa_sigaction = segvhandler;
  action.sa_flags = SA_ONSTACK | SA_RESETHAND | SA_SIGINFO;
  sigemptyset(&action.sa_mask);

  if (sigaction(SIGSEGV, &action, &old_action) == -1) {
    XBT_WARN("Failed to register signal handler for SIGSEGV: %s",
             strerror(errno));
    return;
  }
  if ((old_action.sa_flags & SA_SIGINFO) || old_action.sa_handler != SIG_DFL) {
    XBT_DEBUG("A signal handler was already installed for SIGSEGV (%p). Restore it.",
             (old_action.sa_flags & SA_SIGINFO) ?
             (void*)old_action.sa_sigaction : (void*)old_action.sa_handler);
    sigaction(SIGSEGV, &old_action, NULL);
  }
}

#endif
/********************************* SIMIX **************************************/

XBT_INLINE double SIMIX_timer_next(void)
{
  return xbt_heap_size(simix_timers) > 0 ? xbt_heap_maxkey(simix_timers) : -1.0;
}

/**
 * \ingroup SIMIX_API
 * \brief Initialize SIMIX internal data.
 *
 * \param argc Argc
 * \param argv Argv
 */
void SIMIX_global_init(int *argc, char **argv)
{
  s_smx_process_t proc;

  if (!simix_global) {
    simix_global = xbt_new0(s_smx_global_t, 1);

#ifdef TIME_BENCH_AMDAHL
    simix_global->timer_seq = xbt_os_timer_new();
    simix_global->timer_par = xbt_os_timer_new();
    xbt_os_cputimer_start(simix_global->timer_seq);
#endif
    simix_global->process_to_run = xbt_dynar_new(sizeof(smx_process_t), NULL);
    simix_global->process_that_ran = xbt_dynar_new(sizeof(smx_process_t), NULL);
    simix_global->process_list =
        xbt_swag_new(xbt_swag_offset(proc, process_hookup));
    simix_global->process_to_destroy =
        xbt_swag_new(xbt_swag_offset(proc, destroy_hookup));

    simix_global->maestro_process = NULL;
    simix_global->registered_functions = xbt_dict_new_homogeneous(NULL);

    simix_global->create_process_function = SIMIX_process_create;
    simix_global->kill_process_function = SIMIX_process_kill;
    simix_global->cleanup_process_function = SIMIX_process_cleanup;
    simix_global->synchro_mallocator = xbt_mallocator_new(65536,
        SIMIX_synchro_mallocator_new_f, SIMIX_synchro_mallocator_free_f,
        SIMIX_synchro_mallocator_reset_f);
    simix_global->autorestart = SIMIX_host_restart_processes;

    surf_init(argc, argv);      /* Initialize SURF structures */
    SIMIX_context_mod_init();
    SIMIX_create_maestro_process();

    /* context exception handlers */
    __xbt_running_ctx_fetch = SIMIX_process_get_running_context;
    __xbt_ex_terminate = SIMIX_process_exception_terminate;

    SIMIX_network_init();

    /* Prepare to display some more info when dying on Ctrl-C pressing */
    signal(SIGINT, inthandler);

#ifndef WIN32
    /* Install SEGV handler */
    install_segvhandler();
#endif
    /* register a function to be called by SURF after the environment creation */
    sg_platf_init();
    sg_platf_postparse_add_cb(SIMIX_post_create_environment);

  }
  if (!simix_timers) {
    simix_timers = xbt_heap_new(8, &free);
  }

  SIMIX_HOST_LEVEL = xbt_lib_add_level(host_lib,SIMIX_host_destroy);
  SIMIX_STORAGE_LEVEL = xbt_lib_add_level(storage_lib, SIMIX_storage_destroy);

  if (sg_cfg_get_boolean("clean_atexit"))
    atexit(SIMIX_clean);

  if (_sg_cfg_exit_asap)
    exit(0);
}

/**
 * \ingroup SIMIX_API
 * \brief Clean the SIMIX simulation
 *
 * This functions remove the memory used by SIMIX
 */
void SIMIX_clean(void)
{
  static int cleaned = 0;
#ifdef TIME_BENCH_PER_SR
  smx_ctx_raw_new_sr();
#endif
  if (cleaned) return; // to avoid double cleaning by java and C
  cleaned = 1;
  /* Kill everyone (except maestro) */
  SIMIX_process_killall(simix_global->maestro_process, 1);

  /* Exit the SIMIX network module */
  SIMIX_network_exit();

  xbt_heap_free(simix_timers);
  simix_timers = NULL;
  /* Free the remaining data structures */
  xbt_dynar_free(&simix_global->process_to_run);
  xbt_dynar_free(&simix_global->process_that_ran);
  xbt_swag_free(simix_global->process_to_destroy);
  xbt_swag_free(simix_global->process_list);
  simix_global->process_list = NULL;
  simix_global->process_to_destroy = NULL;
  xbt_dict_free(&(simix_global->registered_functions));

  /* Let's free maestro now */
  SIMIX_context_free(simix_global->maestro_process->context);
  xbt_free(simix_global->maestro_process->running_ctx);
  xbt_free(simix_global->maestro_process);
  simix_global->maestro_process = NULL;

  /* Restore the default exception setup */
  __xbt_running_ctx_fetch = &__xbt_ex_ctx_default;
  __xbt_ex_terminate = &__xbt_ex_terminate_default;

  /* Finish context module and SURF */
  SIMIX_context_mod_exit();

  surf_exit();

#ifdef TIME_BENCH_AMDAHL
  xbt_os_cputimer_stop(simix_global->timer_seq);
  XBT_INFO("Amdahl timing informations. Sequential time: %f; Parallel time: %f",
           xbt_os_timer_elapsed(simix_global->timer_seq),
           xbt_os_timer_elapsed(simix_global->timer_par));
  xbt_os_timer_free(simix_global->timer_seq);
  xbt_os_timer_free(simix_global->timer_par);
#endif

  xbt_mallocator_free(simix_global->synchro_mallocator);
  xbt_free(simix_global);
  simix_global = NULL;

  return;
}


/**
 * \ingroup SIMIX_API
 * \brief A clock (in second).
 *
 * \return Return the clock.
 */
XBT_INLINE double SIMIX_get_clock(void)
{
  if(MC_is_active()){
    return MC_process_clock_get(SIMIX_process_self());
  }else{
    return surf_get_clock();
  }
}

static int process_syscall_color(void *p)
{
  switch ((*(smx_process_t *)p)->simcall.call) {
  case SIMCALL_NONE:
  case SIMCALL_PROCESS_KILL:
    return 2;
  case SIMCALL_PROCESS_RESUME:
    return 1;
  default:
    return 0;
  }
}

/**
 * \ingroup SIMIX_API
 * \brief Run the main simulation loop.
 */
void SIMIX_run(void)
{
  double time = 0;
  smx_process_t process;
  surf_action_t action;
  smx_timer_t timer;
  surf_model_t model;
  unsigned int iter;

  do {
    XBT_DEBUG("New Schedule Round; size(queue)=%lu",
        xbt_dynar_length(simix_global->process_to_run));
#ifdef TIME_BENCH_PER_SR
    smx_ctx_raw_new_sr();
#endif
    while (!xbt_dynar_is_empty(simix_global->process_to_run)) {
      XBT_DEBUG("New Sub-Schedule Round; size(queue)=%lu",
              xbt_dynar_length(simix_global->process_to_run));

      /* Run all processes that are ready to run, possibly in parallel */
#ifdef TIME_BENCH_AMDAHL
      xbt_os_cputimer_stop(simix_global->timer_seq);
      xbt_os_cputimer_resume(simix_global->timer_par);
#endif
      SIMIX_process_runall();
#ifdef TIME_BENCH_AMDAHL
      xbt_os_cputimer_stop(simix_global->timer_par);
      xbt_os_cputimer_resume(simix_global->timer_seq);
#endif

      /* Move all killing processes to the end of the list, because killing a process that have an ongoing simcall is a bad idea */
      xbt_dynar_three_way_partition(simix_global->process_that_ran, process_syscall_color);

      /* answer sequentially and in a fixed arbitrary order all the simcalls that were issued during that sub-round */

      /* WARNING, the order *must* be fixed or you'll jeopardize the simulation reproducibility (see RR-7653) */

      /* Here, the order is ok because:
       *
       *   Short proof: only maestro adds stuff to the process_to_run array, so the execution order of user contexts do not impact its order.
       *
       *   Long proof: processes remain sorted through an arbitrary (implicit, complex but fixed) order in all cases.
       *
       *   - if there is no kill during the simulation, processes remain sorted according by their PID.
       *     rational: This can be proved inductively.
       *        Assume that process_to_run is sorted at a beginning of one round (it is at round 0: the deployment file is parsed linearly).
       *        Let's show that it is still so at the end of this round.
       *        - if a process is added when being created, that's from maestro. It can be either at startup
       *          time (and then in PID order), or in response to a process_create simcall. Since simcalls are handled
       *          in arbitrary order (inductive hypothesis), we are fine.
       *        - If a process is added because it's getting killed, its subsequent actions shouldn't matter
       *        - If a process gets added to process_to_run because one of their blocking action constituting the meat
       *          of a simcall terminates, we're still good. Proof:
       *          - You are added from SIMIX_simcall_answer() only. When this function is called depends on the resource
       *            kind (network, cpu, disk, whatever), but the same arguments hold. Let's take communications as an example.
       *          - For communications, this function is called from SIMIX_comm_finish().
       *            This function itself don't mess with the order since simcalls are handled in FIFO order.
       *            The function is called:
       *            - before the comm starts (invalid parameters, or resource already dead or whatever).
       *              The order then trivial holds since maestro didn't interrupt its handling of the simcall yet
       *            - because the communication failed or were canceled after startup. In this case, it's called from the function
       *              we are in, by the chunk:
       *                       set = model->states.failed_action_set;
       *                       while ((synchro = xbt_swag_extract(set)))
       *                          SIMIX_simcall_post((smx_synchro_t) synchro->data);
       *              This order is also fixed because it depends of the order in which the surf actions were
       *              added to the system, and only maestro can add stuff this way, through simcalls.
       *              We thus use the inductive hypothesis once again to conclude that the order in which synchros are
       *              poped out of the swag does not depend on the user code's execution order.
       *            - because the communication terminated. In this case, synchros are served in the order given by
       *                       set = model->states.done_action_set;
       *                       while ((synchro = xbt_swag_extract(set)))
       *                          SIMIX_simcall_post((smx_synchro_t) synchro->data);
       *              and the argument is very similar to the previous one.
       *            So, in any case, the orders of calls to SIMIX_comm_finish() do not depend on the order in which user processes are executed.
       *          So, in any cases, the orders of processes within process_to_run do not depend on the order in which user processes were executed previously.
       *     So, if there is no killing in the simulation, the simulation reproducibility is not jeopardized.
       *   - If there is some process killings, the order is changed by this decision that comes from user-land
       *     But this decision may not have been motivated by a situation that were different because the simulation is not reproducible.
       *     So, even the order change induced by the process killing is perfectly reproducible.
       *
       *   So science works, bitches [http://xkcd.com/54/].
       *
       *   We could sort the process_that_ran array completely so that we can describe the order in which simcalls are handled
       *   (like "according to the PID of issuer"), but it's not mandatory (order is fixed already even if unfriendly).
       *   That would thus be a pure waste of time.
       */

      xbt_dynar_foreach(simix_global->process_that_ran, iter, process) {
        if (process->simcall.call != SIMCALL_NONE) {
          SIMIX_simcall_handle(&process->simcall, 0);
        }
      }
    }

    time = SIMIX_timer_next();
    if (time != -1.0 || xbt_swag_size(simix_global->process_list) != 0)
      time = surf_solve(time);

    /* Notify all the hosts that have failed */
    /* FIXME: iterate through the list of failed host and mark each of them */
    /* as failed. On each host, signal all the running processes with host_fail */

    /* Handle any pending timer */
    while (xbt_heap_size(simix_timers) > 0 && SIMIX_get_clock() >= SIMIX_timer_next()) {
       //FIXME: make the timers being real callbacks
       // (i.e. provide dispatchers that read and expand the args)
       timer = xbt_heap_pop(simix_timers);
       if (timer->func)
         ((void (*)(void*))timer->func)(timer->args);
       xbt_free(timer);
    }

    /* Wake up all processes waiting for a Surf action to finish */
    xbt_dynar_foreach(model_list, iter, model) {
      while ((action = surf_model_extract_failed_action_set(model)))
        SIMIX_simcall_exit((smx_synchro_t) surf_action_get_data(action));

      while ((action = surf_model_extract_done_action_set(model)))
        if (surf_action_get_data(action) == NULL)
          XBT_DEBUG("probably vcpu's action %p, skip", action);
        else
          SIMIX_simcall_exit((smx_synchro_t) surf_action_get_data(action));
    }

    /* Autorestart all process */
    if(host_that_restart) {
      char *hostname = NULL;
      xbt_dynar_foreach(host_that_restart,iter,hostname) {
        XBT_INFO("Restart processes on host: %s",hostname);
        SIMIX_host_autorestart(SIMIX_host_get_by_name(hostname));
      }
      xbt_dynar_reset(host_that_restart);
    }

    /* Clean processes to destroy */
    SIMIX_process_empty_trash();


    XBT_DEBUG("### time %f, empty %d", time, xbt_dynar_is_empty(simix_global->process_to_run));

  } while (time != -1.0 || !xbt_dynar_is_empty(simix_global->process_to_run));

  if (xbt_swag_size(simix_global->process_list) != 0) {

#ifdef HAVE_TRACING
    TRACE_end();
#endif

    XBT_CRITICAL("Oops ! Deadlock or code not perfectly clean.");
    SIMIX_display_process_status();
    xbt_abort();
  }
}

/**
 *   \brief Set the date to execute a function
 *
 * Set the date to execute the function on the surf.
 *   \param date Date to execute function
 *   \param function Function to be executed
 *   \param arg Parameters of the function
 *
 */
XBT_INLINE void SIMIX_timer_set(double date, void *function, void *arg)
{
  smx_timer_t timer = xbt_new0(s_smx_timer_t, 1);

  timer->date = date;
  timer->func = function;
  timer->args = arg;
  xbt_heap_push(simix_timers, timer, date);
}

/**
 * \brief Registers a function to create a process.
 *
 * This function registers a function to be called
 * when a new process is created. The function has
 * to call SIMIX_process_create().
 * \param function create process function
 */
XBT_INLINE void SIMIX_function_register_process_create(smx_creation_func_t
                                                       function)
{
  simix_global->create_process_function = function;
}

/**
 * \brief Registers a function to kill a process.
 *
 * This function registers a function to be called when a
 * process is killed. The function has to call the SIMIX_process_kill().
 *
 * \param function Kill process function
 */
XBT_INLINE void SIMIX_function_register_process_kill(void_pfn_smxprocess_t_smxprocess_t
                                                     function)
{
  simix_global->kill_process_function = function;
}

/**
 * \brief Registers a function to cleanup a process.
 *
 * This function registers a user function to be called when
 * a process ends properly.
 *
 * \param function cleanup process function
 */
XBT_INLINE void SIMIX_function_register_process_cleanup(void_pfn_smxprocess_t
                                                        function)
{
  simix_global->cleanup_process_function = function;
}


void SIMIX_display_process_status(void)
{
  if (simix_global->process_list == NULL) {
    return;
  }

  smx_process_t process = NULL;
  int nbprocess = xbt_swag_size(simix_global->process_list);

  XBT_INFO("%d processes are still running, waiting for something.", nbprocess);
  /*  List the process and their state */
  XBT_INFO
    ("Legend of the following listing: \"Process <pid> (<name>@<host>): <status>\"");
  xbt_swag_foreach(process, simix_global->process_list) {

    if (process->waiting_synchro) {

      const char* synchro_description = "unknown";
      switch (process->waiting_synchro->type) {

      case SIMIX_SYNC_EXECUTE:
        synchro_description = "execution";
        break;

      case SIMIX_SYNC_PARALLEL_EXECUTE:
        synchro_description = "parallel execution";
        break;

      case SIMIX_SYNC_COMMUNICATE:
        synchro_description = "communication";
        break;

      case SIMIX_SYNC_SLEEP:
        synchro_description = "sleeping";
        break;

      case SIMIX_SYNC_JOIN:
        synchro_description = "joining";
        break;

      case SIMIX_SYNC_SYNCHRO:
        synchro_description = "synchronization";
        break;

      case SIMIX_SYNC_IO:
        synchro_description = "I/O";
        break;
      }
      XBT_INFO("Process %lu (%s@%s): waiting for %s synchro %p (%s) in state %d to finish",
          process->pid, process->name, sg_host_name(process->smx_host),
          synchro_description, process->waiting_synchro,
          process->waiting_synchro->name, (int)process->waiting_synchro->state);
    }
    else {
      XBT_INFO("Process %lu (%s@%s)", process->pid, process->name, sg_host_name(process->smx_host));
    }
  }
}

static void* SIMIX_synchro_mallocator_new_f(void) {
  smx_synchro_t synchro = xbt_new(s_smx_synchro_t, 1);
  synchro->simcalls = xbt_fifo_new();
  return synchro;
}

static void SIMIX_synchro_mallocator_free_f(void* synchro) {
  xbt_fifo_free(((smx_synchro_t) synchro)->simcalls);
  xbt_free(synchro);
}

static void SIMIX_synchro_mallocator_reset_f(void* synchro) {

  // we also recycle the simcall list
  xbt_fifo_t fifo = ((smx_synchro_t) synchro)->simcalls;
  xbt_fifo_reset(fifo);
  memset(synchro, 0, sizeof(s_smx_synchro_t));
  ((smx_synchro_t) synchro)->simcalls = fifo;
}

xbt_dict_t simcall_HANDLER_asr_get_properties(smx_simcall_t simcall, const char *name){
  return SIMIX_asr_get_properties(name);
}
xbt_dict_t SIMIX_asr_get_properties(const char *name)
{
  return xbt_lib_get_or_null(as_router_lib, name, ROUTING_PROP_ASR_LEVEL);
}

