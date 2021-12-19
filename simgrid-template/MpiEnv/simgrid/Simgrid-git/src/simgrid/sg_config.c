/* Copyright (c) 2009-2010, 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/* sg_config: configuration infrastructure for the simulation world       */

#include "xbt/misc.h"
#include "xbt/config.h"
#include "xbt/log.h"
#include "xbt/mallocator.h"
#include "xbt/str.h"
#include "xbt/lib.h"
#include "xbt/sysdep.h"
#include "surf/surf.h"
#include "surf/maxmin.h"
#include "instr/instr_interface.h"
#include "simgrid/simix.h"
#include "simgrid/sg_config.h"
#include "smpi/smpi_interface.h"
#include "mc/mc.h"
#include "instr/instr.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(surf_config, surf,
                                "About the configuration of simgrid");

xbt_cfg_t _sg_cfg_set = NULL;

/* 0: beginning of time (config cannot be changed yet);
 * 1: initialized: cfg_set created (config can now be changed);
 * 2: configured: command line parsed and config part of platform file was
 *    integrated also, platform construction ongoing or done.
 *    (Config cannot be changed anymore!)
 */
int _sg_cfg_init_status = 0;

/* instruct the upper layer (simix or simdag) to exit as soon as possible
 */
int _sg_cfg_exit_asap = 0;

#define sg_cfg_exit_early() do { _sg_cfg_exit_asap = 1; return; } while (0)

/* Parse the command line, looking for options */
static void sg_config_cmd_line(int *argc, char **argv)
{
  int shall_exit = 0;
  int i, j;
  char *opt;

  for (j = i = 1; i < *argc; i++) {
    if (!strncmp(argv[i], "--cfg=", strlen("--cfg="))) {
      opt = strchr(argv[i], '=');
      opt++;

      xbt_cfg_set_parse(_sg_cfg_set, opt);
      XBT_DEBUG("Did apply '%s' as config setting", opt);
    } else if (!strcmp(argv[i], "--version")) {
      printf("%s\n", SIMGRID_VERSION_STRING);
      shall_exit = 1;
    } else if (!strcmp(argv[i], "--cfg-help") || !strcmp(argv[i], "--help")) {
      printf
          ("Description of the configuration accepted by this simulator:\n");
      xbt_cfg_help(_sg_cfg_set);
      printf(
"\n"
"Each of these configurations can be used by adding\n"
"    --cfg=<option name>:<option value>\n"
"to the command line.\n"
"\n"
"You can also use --help-models to see the details of all models known by this simulator.\n"
#ifdef HAVE_TRACING
"\n"
"You can also use --help-tracing to see the details of all tracing options known by this simulator.\n"
#endif
"\n"
"You can also use --help-logs and --help-log-categories to see the details of logging output.\n"
"\n"
"You can also use --version to get SimGrid version information.\n"
"\n"
        );
      shall_exit = 1;
    } else if (!strcmp(argv[i], "--help-models")) {
      int k;

      model_help("workstation", surf_workstation_model_description);
      printf("\n");
      model_help("CPU", surf_cpu_model_description);
      printf("\n");
      model_help("network", surf_network_model_description);
      printf("\nLong description of all optimization levels accepted by the models of this simulator:\n");
      for (k = 0; surf_optimization_mode_description[k].name; k++)
        printf("  %s: %s\n",
               surf_optimization_mode_description[k].name,
               surf_optimization_mode_description[k].description);
      printf("Both network and CPU models have 'Lazy' as default optimization level\n\n");
      shall_exit = 1;
#ifdef HAVE_TRACING
    } else if (!strcmp(argv[i], "--help-tracing")) {
      TRACE_help (1);
      shall_exit = 1;
#endif
    } else {
      argv[j++] = argv[i];
    }
  }
  if (j < *argc) {
    argv[j] = NULL;
    *argc = j;
  }
  if (shall_exit)
    sg_cfg_exit_early();
}

/* callback of the plugin variable */
static void _sg_cfg_cb__plugin(const char *name, int pos)
{
  char *val;

  XBT_VERB("PLUGIN");
  xbt_assert(_sg_cfg_init_status < 2,
              "Cannot load a plugin after the initialization");

  val = xbt_cfg_get_string(_sg_cfg_set, name);

  if (!strcmp(val, "help")) {
    model_help("plugin", surf_plugin_description);
    sg_cfg_exit_early();
  }

  /* New Module missing */
  int plugin_id = find_model_description(surf_plugin_description, val);
  surf_plugin_description[plugin_id].model_init_preparse();
}

/* callback of the workstation/model variable */
static void _sg_cfg_cb__workstation_model(const char *name, int pos)
{
  char *val;

  xbt_assert(_sg_cfg_init_status < 2,
              "Cannot change the model after the initialization");

  val = xbt_cfg_get_string(_sg_cfg_set, name);

  if (!strcmp(val, "help")) {
    model_help("workstation", surf_workstation_model_description);
    sg_cfg_exit_early();
  }

  /* Make sure that the model exists */
  find_model_description(surf_workstation_model_description, val);
}

/* callback of the vm_workstation/model variable */
static void _sg_cfg_cb__vm_workstation_model(const char *name, int pos)
{
  char *val;

  xbt_assert(_sg_cfg_init_status < 2,
              "Cannot change the model after the initialization");

  val = xbt_cfg_get_string(_sg_cfg_set, name);

  if (!strcmp(val, "help")) {
    model_help("vm_workstation", surf_vm_workstation_model_description);
    sg_cfg_exit_early();
  }

  /* Make sure that the model exists */
  find_model_description(surf_vm_workstation_model_description, val);
}

/* callback of the cpu/model variable */
static void _sg_cfg_cb__cpu_model(const char *name, int pos)
{
  char *val;

  xbt_assert(_sg_cfg_init_status < 2,
              "Cannot change the model after the initialization");

  val = xbt_cfg_get_string(_sg_cfg_set, name);

  if (!strcmp(val, "help")) {
    model_help("CPU", surf_cpu_model_description);
    sg_cfg_exit_early();
  }

  /* New Module missing */
  find_model_description(surf_cpu_model_description, val);
}

/* callback of the cpu/model variable */
static void _sg_cfg_cb__optimization_mode(const char *name, int pos)
{
  char *val;

  xbt_assert(_sg_cfg_init_status < 2,
              "Cannot change the model after the initialization");

  val = xbt_cfg_get_string(_sg_cfg_set, name);

  if (!strcmp(val, "help")) {
    model_help("optimization", surf_optimization_mode_description);
    sg_cfg_exit_early();
  }

  /* New Module missing */
  find_model_description(surf_optimization_mode_description, val);
}

/* callback of the cpu/model variable */
static void _sg_cfg_cb__storage_mode(const char *name, int pos)
{
  char *val;

  xbt_assert(_sg_cfg_init_status < 2,
              "Cannot change the model after the initialization");

  val = xbt_cfg_get_string(_sg_cfg_set, name);

  if (!strcmp(val, "help")) {
    model_help("storage", surf_storage_model_description);
    sg_cfg_exit_early();
  }

  /* New Module missing */
  find_model_description(surf_storage_model_description, val);
}

/* callback of the workstation_model variable */
static void _sg_cfg_cb__network_model(const char *name, int pos)
{
  char *val;

  xbt_assert(_sg_cfg_init_status < 2,
              "Cannot change the model after the initialization");

  val = xbt_cfg_get_string(_sg_cfg_set, name);

  if (!strcmp(val, "help")) {
    model_help("network", surf_network_model_description);
    sg_cfg_exit_early();
  }

  /* New Module missing */
  find_model_description(surf_network_model_description, val);
}

/* callbacks of the network models values */
static void _sg_cfg_cb__tcp_gamma(const char *name, int pos)
{
  sg_tcp_gamma = xbt_cfg_get_double(_sg_cfg_set, name);
}

static void _sg_cfg_cb__maxmin_precision(const char* name, int pos)
{
  sg_maxmin_precision = xbt_cfg_get_double(_sg_cfg_set, name);
}

static void _sg_cfg_cb__surf_precision(const char* name, int pos)
{
  sg_surf_precision = xbt_cfg_get_double(_sg_cfg_set, name);
}

static void _sg_cfg_cb__sender_gap(const char* name, int pos)
{
  sg_sender_gap = xbt_cfg_get_double(_sg_cfg_set, name);
}

static void _sg_cfg_cb__latency_factor(const char *name, int pos)
{
  sg_latency_factor = xbt_cfg_get_double(_sg_cfg_set, name);
}

static void _sg_cfg_cb__bandwidth_factor(const char *name, int pos)
{
  sg_bandwidth_factor = xbt_cfg_get_double(_sg_cfg_set, name);
}

static void _sg_cfg_cb__weight_S(const char *name, int pos)
{
  sg_weight_S_parameter = xbt_cfg_get_double(_sg_cfg_set, name);
}

#ifdef HAVE_SMPI
/* callback of the mpi collectives */
static void _sg_cfg_cb__coll(const char *category,
                             s_mpi_coll_description_t * table,
                             const char *name, int pos)
{
  char *val;

  xbt_assert(_sg_cfg_init_status < 2,
              "Cannot change the model after the initialization");

  val = xbt_cfg_get_string(_sg_cfg_set, name);

  if (!strcmp(val, "help")) {
    coll_help(category, table);
    sg_cfg_exit_early();
  }

  /* New Module missing */
  find_coll_description(table, val);
}
static void _sg_cfg_cb__coll_gather(const char *name, int pos){
  _sg_cfg_cb__coll("gather", mpi_coll_gather_description, name, pos);
}
static void _sg_cfg_cb__coll_allgather(const char *name, int pos){
  _sg_cfg_cb__coll("allgather", mpi_coll_allgather_description, name, pos);
}
static void _sg_cfg_cb__coll_allgatherv(const char *name, int pos){
  _sg_cfg_cb__coll("allgatherv", mpi_coll_allgatherv_description, name, pos);
}
static void _sg_cfg_cb__coll_allreduce(const char *name, int pos)
{
  _sg_cfg_cb__coll("allreduce", mpi_coll_allreduce_description, name, pos);
}
static void _sg_cfg_cb__coll_alltoall(const char *name, int pos)
{
  _sg_cfg_cb__coll("alltoall", mpi_coll_alltoall_description, name, pos);
}
static void _sg_cfg_cb__coll_alltoallv(const char *name, int pos)
{
  _sg_cfg_cb__coll("alltoallv", mpi_coll_alltoallv_description, name, pos);
}
static void _sg_cfg_cb__coll_bcast(const char *name, int pos)
{
  _sg_cfg_cb__coll("bcast", mpi_coll_bcast_description, name, pos);
}
static void _sg_cfg_cb__coll_reduce(const char *name, int pos)
{
  _sg_cfg_cb__coll("reduce", mpi_coll_reduce_description, name, pos);
}
static void _sg_cfg_cb__coll_reduce_scatter(const char *name, int pos){
  _sg_cfg_cb__coll("reduce_scatter", mpi_coll_reduce_scatter_description, name, pos);
}
static void _sg_cfg_cb__coll_scatter(const char *name, int pos){
  _sg_cfg_cb__coll("scatter", mpi_coll_scatter_description, name, pos);
}
static void _sg_cfg_cb__coll_barrier(const char *name, int pos){
  _sg_cfg_cb__coll("barrier", mpi_coll_barrier_description, name, pos);
}

static void _sg_cfg_cb__wtime_sleep(const char *name, int pos){
  smpi_wtime_sleep = xbt_cfg_get_double(_sg_cfg_set, name);
}

static void _sg_cfg_cb__iprobe_sleep(const char *name, int pos){
  smpi_iprobe_sleep = xbt_cfg_get_double(_sg_cfg_set, name);
}

static void _sg_cfg_cb__test_sleep(const char *name, int pos){
  smpi_test_sleep = xbt_cfg_get_double(_sg_cfg_set, name);
}



#endif

/* callback of the inclusion path */
static void _sg_cfg_cb__surf_path(const char *name, int pos)
{
  char *path = xbt_cfg_get_string_at(_sg_cfg_set, name, pos);
  xbt_dynar_push(surf_path, &path);
}

/* callback to decide if we want to use the model-checking */
#include "xbt_modinter.h"
#ifdef HAVE_MC
extern int _sg_do_model_check;   /* this variable lives in xbt_main until I find a right location for it */
#endif

static void _sg_cfg_cb_model_check(const char *name, int pos)
{
#ifdef HAVE_MC
  _sg_do_model_check = xbt_cfg_get_boolean(_sg_cfg_set, name);
#else
  if (xbt_cfg_get_boolean(_sg_cfg_set, name)) {
    xbt_die("You tried to activate the model-checking from the command line, but it was not compiled in. Change your settings in cmake, recompile and try again");
  }
#endif
}

extern int _sg_do_verbose_exit;

static void _sg_cfg_cb_verbose_exit(const char *name, int pos)
{
  _sg_do_verbose_exit = xbt_cfg_get_boolean(_sg_cfg_set, name);
}

extern int _sg_do_clean_atexit;

static void _sg_cfg_cb_clean_atexit(const char *name, int pos)
{
  _sg_do_clean_atexit = xbt_cfg_get_boolean(_sg_cfg_set, name);
}

static void _sg_cfg_cb_context_factory(const char *name, int pos)
{
  smx_context_factory_name = xbt_cfg_get_string(_sg_cfg_set, name);
}

static void _sg_cfg_cb_context_stack_size(const char *name, int pos)
{
  smx_context_stack_size_was_set = 1;
  smx_context_stack_size = xbt_cfg_get_int(_sg_cfg_set, name) * 1024;
}

static void _sg_cfg_cb_context_guard_size(const char *name, int pos)
{
  smx_context_guard_size_was_set = 1;
  smx_context_guard_size = xbt_cfg_get_int(_sg_cfg_set, name) * xbt_pagesize;
}

static void _sg_cfg_cb_contexts_nthreads(const char *name, int pos)
{
  SIMIX_context_set_nthreads(xbt_cfg_get_int(_sg_cfg_set, name));
}

static void _sg_cfg_cb_contexts_parallel_threshold(const char *name, int pos)
{
  SIMIX_context_set_parallel_threshold(xbt_cfg_get_int(_sg_cfg_set, name));
}

static void _sg_cfg_cb_contexts_parallel_mode(const char *name, int pos)
{
  const char* mode_name = xbt_cfg_get_string(_sg_cfg_set, name);
  if (!strcmp(mode_name, "posix")) {
    SIMIX_context_set_parallel_mode(XBT_PARMAP_POSIX);
  }
  else if (!strcmp(mode_name, "futex")) {
    SIMIX_context_set_parallel_mode(XBT_PARMAP_FUTEX);
  }
  else if (!strcmp(mode_name, "busy_wait")) {
    SIMIX_context_set_parallel_mode(XBT_PARMAP_BUSY_WAIT);
  }
  else {
    xbt_die("Command line setting of the parallel synchronization mode should "
            "be one of \"posix\", \"futex\" or \"busy_wait\"");
  }
}

static void _sg_cfg_cb__surf_network_coordinates(const char *name,
                                                 int pos)
{
  int val = xbt_cfg_get_boolean(_sg_cfg_set, name);
  if (val) {
    if (!COORD_HOST_LEVEL) {
      COORD_HOST_LEVEL = xbt_lib_add_level(host_lib,xbt_dynar_free_voidp);
      COORD_ASR_LEVEL  = xbt_lib_add_level(as_router_lib,xbt_dynar_free_voidp);
    }
  } else
    if (COORD_HOST_LEVEL)
      xbt_die("Setting of whether to use coordinate cannot be disabled once set.");
}

static void _sg_cfg_cb__surf_network_crosstraffic(const char *name,
                                                  int pos)
{
  sg_network_crosstraffic = xbt_cfg_get_boolean(_sg_cfg_set, name);
}

#ifdef HAVE_GTNETS
static void _sg_cfg_cb__gtnets_jitter(const char *name, int pos)
{
  sg_gtnets_jitter = xbt_cfg_get_double(_sg_cfg_set, name);
}

static void _sg_cfg_cb__gtnets_jitter_seed(const char *name, int pos)
{
  sg_gtnets_jitter_seed = xbt_cfg_get_int(_sg_cfg_set, name);
}
#endif

/* build description line with possible values */
static void describe_model(char *result,
                           const s_surf_model_description_t model_description[],
                           const char *name,
                           const char *description)
{
  char *p = result +
    sprintf(result, "%s. Possible values: %s", description,
            model_description[0].name ? model_description[0].name : "n/a");
  int i;
  for (i = 1; model_description[i].name; i++)
    p += sprintf(p, ", %s", model_description[i].name);
  sprintf(p,
      ".\n       (use 'help' as a value to see the long description of each %s)",
          name);
}

/* create the config set, register what should be and parse the command line*/
void sg_config_init(int *argc, char **argv)
{
  char description[1024];

  /* Create the configuration support */
  if (_sg_cfg_init_status == 0) { /* Only create stuff if not already inited */

    /* Plugins configuration */
    describe_model(description, surf_plugin_description,
                   "plugin", "The plugins");
    xbt_cfg_register(&_sg_cfg_set, "plugin", description,
                     xbt_cfgelm_string, 0, 1, &_sg_cfg_cb__plugin, NULL);

    describe_model(description, surf_cpu_model_description,
                   "model", "The model to use for the CPU");
    xbt_cfg_register(&_sg_cfg_set, "cpu/model", description,
                     xbt_cfgelm_string, 1, 1, &_sg_cfg_cb__cpu_model, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "cpu/model", "Cas01");

    describe_model(description, surf_optimization_mode_description,
                   "optimization mode",
                   "The optimization modes to use for the CPU");
    xbt_cfg_register(&_sg_cfg_set, "cpu/optim", description,
                     xbt_cfgelm_string, 1, 1, &_sg_cfg_cb__optimization_mode, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "cpu/optim", "Lazy");

    describe_model(description, surf_storage_model_description,
                   "model", "The model to use for the storage");
    xbt_cfg_register(&_sg_cfg_set, "storage/model", description,
                     xbt_cfgelm_string, 1, 1, &_sg_cfg_cb__storage_mode, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "storage/model", "default");

    describe_model(description, surf_network_model_description,
                   "model", "The model to use for the network");
    xbt_cfg_register(&_sg_cfg_set, "network/model", description,
                     xbt_cfgelm_string, 1, 1, &_sg_cfg_cb__network_model, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "network/model", "LV08");

    describe_model(description, surf_optimization_mode_description,
                   "optimization mode",
                   "The optimization modes to use for the network");
    xbt_cfg_register(&_sg_cfg_set, "network/optim", description,
                     xbt_cfgelm_string, 1, 1, &_sg_cfg_cb__optimization_mode, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "network/optim", "Lazy");

    describe_model(description, surf_workstation_model_description,
                   "model", "The model to use for the workstation");
    xbt_cfg_register(&_sg_cfg_set, "workstation/model", description,
                     xbt_cfgelm_string, 1, 1, &_sg_cfg_cb__workstation_model, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "workstation/model", "default");

    describe_model(description, surf_vm_workstation_model_description,
                   "model", "The model to use for the vm workstation");
    xbt_cfg_register(&_sg_cfg_set, "vm_workstation/model", description,
                     xbt_cfgelm_string, 1, 1, &_sg_cfg_cb__vm_workstation_model, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "vm_workstation/model", "default");

    xbt_cfg_register(&_sg_cfg_set, "network/TCP_gamma",
                     "Size of the biggest TCP window (cat /proc/sys/net/ipv4/tcp_[rw]mem for recv/send window; Use the last given value, which is the max window size)",
                     xbt_cfgelm_double, 1, 1, _sg_cfg_cb__tcp_gamma, NULL);
    xbt_cfg_setdefault_double(_sg_cfg_set, "network/TCP_gamma", 4194304.0);

    xbt_cfg_register(&_sg_cfg_set, "surf/precision",
                     "Numerical precision used when updating simulation times (hence this value is expressed in seconds)",
                     xbt_cfgelm_double, 1, 1, _sg_cfg_cb__surf_precision, NULL);
    xbt_cfg_setdefault_double(_sg_cfg_set, "surf/precision", 0.00001);

    xbt_cfg_register(&_sg_cfg_set, "maxmin/precision",
                     "Numerical precision used when computing resource sharing (hence this value is expressed in ops/sec or bytes/sec)",
                     xbt_cfgelm_double, 1, 1, _sg_cfg_cb__maxmin_precision, NULL);
    xbt_cfg_setdefault_double(_sg_cfg_set, "maxmin/precision", 0.00001);

    /* The parameters of network models */

    xbt_cfg_register(&_sg_cfg_set, "network/sender_gap",
                     "Minimum gap between two overlapping sends",
                     xbt_cfgelm_double, 1, 1, _sg_cfg_cb__sender_gap, NULL);
    /* real default for "network/sender_gap" is set in network_smpi.cpp */
    xbt_cfg_setdefault_double(_sg_cfg_set, "network/sender_gap", NAN);

    xbt_cfg_register(&_sg_cfg_set, "network/latency_factor",
                     "Correction factor to apply to the provided latency (default value set by network model)",
                     xbt_cfgelm_double, 1, 1, _sg_cfg_cb__latency_factor, NULL);
    xbt_cfg_setdefault_double(_sg_cfg_set, "network/latency_factor", 1.0);

    xbt_cfg_register(&_sg_cfg_set, "network/bandwidth_factor",
                     "Correction factor to apply to the provided bandwidth (default value set by network model)",
                     xbt_cfgelm_double, 1, 1, _sg_cfg_cb__bandwidth_factor, NULL);
    xbt_cfg_setdefault_double(_sg_cfg_set, "network/bandwidth_factor", 1.0);

    xbt_cfg_register(&_sg_cfg_set, "network/weight_S",
                     "Correction factor to apply to the weight of competing streams (default value set by network model)",
                     xbt_cfgelm_double, 1, 1, _sg_cfg_cb__weight_S, NULL);
    /* real default for "network/weight_S" is set in network_*.cpp */
    xbt_cfg_setdefault_double(_sg_cfg_set, "network/weight_S", NAN);

    /* Inclusion path */
    xbt_cfg_register(&_sg_cfg_set, "path",
                     "Lookup path for inclusions in platform and deployment XML files",
                     xbt_cfgelm_string, 1, 0, _sg_cfg_cb__surf_path, NULL);

    xbt_cfg_register(&_sg_cfg_set, "cpu/maxmin_selective_update",
                     "Update the constraint set propagating recursively to others constraints (off by default when optim is set to lazy)",
                     xbt_cfgelm_boolean, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "cpu/maxmin_selective_update", "no");

    xbt_cfg_register(&_sg_cfg_set, "network/maxmin_selective_update",
                     "Update the constraint set propagating recursively to others constraints (off by default when optim is set to lazy)",
                     xbt_cfgelm_boolean, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "network/maxmin_selective_update", "no");

#ifdef HAVE_MC
    /* do model-checking */
    xbt_cfg_register(&_sg_cfg_set, "model-check",
                     "Verify the system through model-checking instead of simulating it (EXPERIMENTAL)",
                     xbt_cfgelm_boolean, 1, 1, _sg_cfg_cb_model_check, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "model-check", "no");

    /* do stateful model-checking */
    xbt_cfg_register(&_sg_cfg_set, "model-check/checkpoint",
                     "Specify the amount of steps between checkpoints during stateful model-checking (default: 0 => stateless verification). "
                     "If value=1, one checkpoint is saved for each step => faster verification, but huge memory consumption; higher values are good compromises between speed and memory consumption.",
                     xbt_cfgelm_int, 1, 1, _mc_cfg_cb_checkpoint, NULL);
    xbt_cfg_setdefault_int(_sg_cfg_set, "model-check/checkpoint", 0);

    /* do stateful model-checking */
    xbt_cfg_register(&_sg_cfg_set, "model-check/sparse-checkpoint",
                     "Use sparse per-page snapshots.",
                     xbt_cfgelm_boolean, 1, 1, _mc_cfg_cb_sparse_checkpoint, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "model-check/sparse-checkpoint", "no");

    /* do stateful model-checking */
    xbt_cfg_register(&_sg_cfg_set, "model-check/soft-dirty",
                     "Use sparse per-page snapshots.",
                     xbt_cfgelm_boolean, 1, 1, _mc_cfg_cb_soft_dirty, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "model-check/soft-dirty", "no");

    /* do liveness model-checking */
    xbt_cfg_register(&_sg_cfg_set, "model-check/property",
                     "Specify the name of the file containing the property. It must be the result of the ltl2ba program.",
                     xbt_cfgelm_string, 1, 1, _mc_cfg_cb_property, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "model-check/property", "");

    /* do communications determinism model-checking */
    xbt_cfg_register(&_sg_cfg_set, "model-check/communications_determinism",
                     "Enable/disable the detection of determinism in the communications schemes",
                     xbt_cfgelm_boolean, 1, 1, _mc_cfg_cb_comms_determinism, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "model-check/communications_determinism", "no");

    /* do send determinism model-checking */
    xbt_cfg_register(&_sg_cfg_set, "model-check/send_determinism",
                     "Enable/disable the detection of send-determinism in the communications schemes",
                     xbt_cfgelm_boolean, 1, 1, _mc_cfg_cb_send_determinism, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "model-check/send_determinism", "no");

    /* Specify the kind of model-checking reduction */
    xbt_cfg_register(&_sg_cfg_set, "model-check/reduction",
                     "Specify the kind of exploration reduction (either none or DPOR)",
                     xbt_cfgelm_string, 1, 1, _mc_cfg_cb_reduce, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "model-check/reduction", "dpor");

    /* Enable/disable timeout for wait requests with model-checking */
    xbt_cfg_register(&_sg_cfg_set, "model-check/timeout",
                     "Enable/Disable timeout for wait requests",
                     xbt_cfgelm_boolean, 1, 1, _mc_cfg_cb_timeout, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "model-check/timeout", "no");

    /* Enable/disable global hash computation with model-checking */
    xbt_cfg_register(&_sg_cfg_set, "model-check/hash",
                     "Enable/Disable state hash for state comparison (exprimental)",
                     xbt_cfgelm_boolean, 1, 1, _mc_cfg_cb_hash, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "model-check/hash", "no");

    /* Set max depth exploration */
    xbt_cfg_register(&_sg_cfg_set, "model-check/max_depth",
                     "Specify the max depth of exploration (default : 1000)",
                     xbt_cfgelm_int, 1, 1, _mc_cfg_cb_max_depth, NULL);
    xbt_cfg_setdefault_int(_sg_cfg_set, "model-check/max_depth", 1000);

    /* Set number of visited state stored for state comparison reduction*/
    xbt_cfg_register(&_sg_cfg_set, "model-check/visited",
                     "Specify the number of visited state stored for state comparison reduction. If value=5, the last 5 visited states are stored",
                     xbt_cfgelm_int, 1, 1, _mc_cfg_cb_visited, NULL);
    xbt_cfg_setdefault_int(_sg_cfg_set, "model-check/visited", 0);

    /* Set file name for dot output of graph state */
    xbt_cfg_register(&_sg_cfg_set, "model-check/dot_output",
                     "Specify the name of dot file corresponding to graph state",
                     xbt_cfgelm_string, 1, 1, _mc_cfg_cb_dot_output, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "model-check/dot_output", "");
#endif

    /* do verbose-exit */
    xbt_cfg_register(&_sg_cfg_set, "verbose-exit",
                     "Activate the \"do nothing\" mode in Ctrl-C",
                     xbt_cfgelm_boolean, 1, 1, _sg_cfg_cb_verbose_exit, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "verbose-exit", "yes");

    /* context factory */
    const char *dflt_ctx_fact = "thread";
    {
      char *p = description +
        sprintf(description,
                "Context factory to use in SIMIX. Possible values: %s",
                dflt_ctx_fact);
#ifdef CONTEXT_UCONTEXT
      dflt_ctx_fact = "ucontext";
      p += sprintf(p, ", %s", dflt_ctx_fact);
#endif
#ifdef HAVE_RAWCTX
      dflt_ctx_fact = "raw";
      p += sprintf(p, ", %s", dflt_ctx_fact);
#endif
      sprintf(p, ".");
    }
    xbt_cfg_register(&_sg_cfg_set, "contexts/factory", description,
                     xbt_cfgelm_string, 1, 1, _sg_cfg_cb_context_factory, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "contexts/factory", dflt_ctx_fact);

    /* stack size of contexts in KiB */
    xbt_cfg_register(&_sg_cfg_set, "contexts/stack_size",
                     "Stack size of contexts in KiB",
                     xbt_cfgelm_int, 1, 1, _sg_cfg_cb_context_stack_size, NULL);
    xbt_cfg_setdefault_int(_sg_cfg_set, "contexts/stack_size", 8*1024);
    /* No, it was not set yet (the above setdefault() changed this to 1). */
    smx_context_stack_size_was_set = 0;

    /* guard size for contexts stacks in memory pages */
    xbt_cfg_register(&_sg_cfg_set, "contexts/guard_size",
                     "Guard size for contexts stacks in memory pages",
                     xbt_cfgelm_int, 1, 1, _sg_cfg_cb_context_guard_size, NULL);
#if defined(_XBT_WIN32) || (PTH_STACKGROWTH != -1)
    xbt_cfg_setdefault_int(_sg_cfg_set, "contexts/guard_size", 0);
#else
    xbt_cfg_setdefault_int(_sg_cfg_set, "contexts/guard_size", 1);
#endif
    /* No, it was not set yet (the above setdefault() changed this to 1). */
    smx_context_guard_size_was_set = 0;

    /* number of parallel threads for user processes */
    xbt_cfg_register(&_sg_cfg_set, "contexts/nthreads",
                     "Number of parallel threads used to execute user contexts",
                     xbt_cfgelm_int, 1, 1, _sg_cfg_cb_contexts_nthreads, NULL);
    xbt_cfg_setdefault_int(_sg_cfg_set, "contexts/nthreads", 1);

    /* minimal number of user contexts to be run in parallel */
    xbt_cfg_register(&_sg_cfg_set, "contexts/parallel_threshold",
                     "Minimal number of user contexts to be run in parallel (raw contexts only)",
                     xbt_cfgelm_int, 1, 1, _sg_cfg_cb_contexts_parallel_threshold, NULL);
    xbt_cfg_setdefault_int(_sg_cfg_set, "contexts/parallel_threshold", 2);

    /* synchronization mode for parallel user contexts */
    xbt_cfg_register(&_sg_cfg_set, "contexts/synchro",
                     "Synchronization mode to use when running contexts in parallel (either futex, posix or busy_wait)",
                     xbt_cfgelm_string, 1, 1, _sg_cfg_cb_contexts_parallel_mode, NULL);
#ifdef HAVE_FUTEX_H
    xbt_cfg_setdefault_string(_sg_cfg_set, "contexts/synchro", "futex");
#else //No futex on mac and posix is unimplememted yet
    xbt_cfg_setdefault_string(_sg_cfg_set, "contexts/synchro", "busy_wait");
#endif

    xbt_cfg_register(&_sg_cfg_set, "network/coordinates",
                     "\"yes\" or \"no\", specifying whether we use a coordinate-based routing (as Vivaldi)",
                     xbt_cfgelm_boolean, 1, 1, _sg_cfg_cb__surf_network_coordinates, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "network/coordinates", "no");

    xbt_cfg_register(&_sg_cfg_set, "network/crosstraffic",
                     "Activate the interferences between uploads and downloads for fluid max-min models (LV08, CM02)",
                     xbt_cfgelm_boolean, 1, 1, _sg_cfg_cb__surf_network_crosstraffic, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "network/crosstraffic", "no");

#ifdef HAVE_GTNETS
    xbt_cfg_register(&_sg_cfg_set, "gtnets/jitter",
                     "Double value to oscillate the link latency, uniformly in random interval [-latency*gtnets_jitter,latency*gtnets_jitter)",
                     xbt_cfgelm_double, 1, 1, _sg_cfg_cb__gtnets_jitter, NULL);
    xbt_cfg_setdefault_double(_sg_cfg_set, "gtnets/jitter", 0.0);

    xbt_cfg_register(&_sg_cfg_set, "gtnets/jitter_seed",
                     "Use a positive seed to reproduce jitted results, value must be in [1,1e8], default is 10",
                     xbt_cfgelm_int, 0, 1, _sg_cfg_cb__gtnets_jitter_seed, NULL);
    xbt_cfg_setdefault_int(_sg_cfg_set, "gtnets/jitter_seed", 10);
#endif
#ifdef HAVE_NS3
    xbt_cfg_register(&_sg_cfg_set, "ns3/TcpModel",
                     "The ns3 tcp model can be : NewReno or Reno or Tahoe",
                     xbt_cfgelm_string, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "ns3/TcpModel", "default");
#endif

#ifdef HAVE_SMPI
    xbt_cfg_register(&_sg_cfg_set, "smpi/running_power",
                     "Power of the host running the simulation (in flop/s). Used to bench the operations.",
                     xbt_cfgelm_double, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_double(_sg_cfg_set, "smpi/running_power", 20000.0);

    xbt_cfg_register(&_sg_cfg_set, "smpi/display_timing",
                     "Boolean indicating whether we should display the timing after simulation.",
                     xbt_cfgelm_boolean, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "smpi/display_timing", "no");

    xbt_cfg_register(&_sg_cfg_set, "smpi/simulate_computation",
                     "Boolean indicating whether the computational part of the simulated application should be simulated.",
                     xbt_cfgelm_boolean, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "smpi/simulate_computation", "yes");

    xbt_cfg_register(&_sg_cfg_set, "smpi/use_shared_malloc",
                     "Boolean indicating whether we should use shared memory when using SMPI_SHARED_MALLOC. Allows user to disable it for debug purposes.",
                     xbt_cfgelm_boolean, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "smpi/use_shared_malloc", "yes");

    xbt_cfg_register(&_sg_cfg_set, "smpi/cpu_threshold",
                     "Minimal computation time (in seconds) not discarded, or -1 for infinity.",
                     xbt_cfgelm_double, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_double(_sg_cfg_set, "smpi/cpu_threshold", 1e-6);

    xbt_cfg_register(&_sg_cfg_set, "smpi/async_small_thres",
                     "Maximal size of messages that are to be sent asynchronously, without waiting for the receiver",
                     xbt_cfgelm_int, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_int(_sg_cfg_set, "smpi/async_small_thres", 0);

    xbt_cfg_register(&_sg_cfg_set, "smpi/send_is_detached_thres",
                     "Threshold of message size where MPI_Send stops behaving like MPI_Isend and becomes MPI_Ssend",
                     xbt_cfgelm_int, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_int(_sg_cfg_set, "smpi/send_is_detached_thres", 65536);

    xbt_cfg_register(&_sg_cfg_set, "smpi/privatize_global_variables",
                     "Boolean indicating whether we should privatize global variable at runtime.",
                     xbt_cfgelm_boolean, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "smpi/privatize_global_variables", "no");

    //For smpi/bw_factor and smpi/lat_factor
    //Default value have to be "threshold0:value0;threshold1:value1;...;thresholdN:valueN"
    //test is if( size >= thresholdN ) return valueN;
    //Values can be modified with command line --cfg=smpi/bw_factor:"threshold0:value0;threshold1:value1;...;thresholdN:valueN"
    //  or with tag config put line <prop id="smpi/bw_factor" value="threshold0:value0;threshold1:value1;...;thresholdN:valueN"></prop>
    xbt_cfg_register(&_sg_cfg_set, "smpi/bw_factor",
                     "Bandwidth factors for smpi.",
                     xbt_cfgelm_string, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "smpi/bw_factor", "65472:0.940694;15424:0.697866;9376:0.58729;5776:1.08739;3484:0.77493;1426:0.608902;732:0.341987;257:0.338112;0:0.812084");

    xbt_cfg_register(&_sg_cfg_set, "smpi/lat_factor",
                     "Latency factors for smpi.",
                     xbt_cfgelm_string, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "smpi/lat_factor", "65472:11.6436;15424:3.48845;9376:2.59299;5776:2.18796;3484:1.88101;1426:1.61075;732:1.9503;257:1.95341;0:2.01467");

    xbt_cfg_register(&_sg_cfg_set, "smpi/IB_penalty_factors",
                     "Correction factor to communications using Infiniband model with contention (default value based on Stampede cluster profiling)",
                     xbt_cfgelm_string, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "smpi/IB_penalty_factors", "0.965;0.925;1.35");

    xbt_cfg_register(&_sg_cfg_set, "smpi/os",
                     "Small messages timings (MPI_Send minimum time for small messages)",
                     xbt_cfgelm_string, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "smpi/os", "1:0:0:0:0");

    xbt_cfg_register(&_sg_cfg_set, "smpi/ois",
                     "Small messages timings (MPI_Isend minimum time for small messages)",
                     xbt_cfgelm_string, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "smpi/ois", "1:0:0:0:0");

    xbt_cfg_register(&_sg_cfg_set, "smpi/or",
                     "Small messages timings (MPI_Recv minimum time for small messages)",
                     xbt_cfgelm_string, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "smpi/or", "1:0:0:0:0");

    xbt_cfg_register(&_sg_cfg_set, "smpi/iprobe",
                     "Minimum time to inject inside a call to MPI_Iprobe",
                     xbt_cfgelm_double, 1, 1, _sg_cfg_cb__iprobe_sleep, NULL);
    xbt_cfg_setdefault_double(_sg_cfg_set, "smpi/iprobe", 1e-4);

    xbt_cfg_register(&_sg_cfg_set, "smpi/test",
                     "Minimum time to inject inside a call to MPI_Test",
                     xbt_cfgelm_double, 1, 1, _sg_cfg_cb__test_sleep, NULL);
    xbt_cfg_setdefault_double(_sg_cfg_set, "smpi/test", 1e-4);

    xbt_cfg_register(&_sg_cfg_set, "smpi/wtime",
                     "Minimum time to inject inside a call to MPI_Wtime",
                     xbt_cfgelm_double, 1, 1, _sg_cfg_cb__wtime_sleep, NULL);
    xbt_cfg_setdefault_double(_sg_cfg_set, "smpi/wtime", 0.0);

    xbt_cfg_register(&_sg_cfg_set, "smpi/coll_selector",
                     "Which collective selector to use",
                     xbt_cfgelm_string, 1, 1, NULL, NULL);
    xbt_cfg_setdefault_string(_sg_cfg_set, "smpi/coll_selector", "default");

    xbt_cfg_register(&_sg_cfg_set, "smpi/gather",
                     "Which collective to use for gather",
                     xbt_cfgelm_string, 0, 1, &_sg_cfg_cb__coll_gather, NULL);

    xbt_cfg_register(&_sg_cfg_set, "smpi/allgather",
                     "Which collective to use for allgather",
                     xbt_cfgelm_string, 0, 1, &_sg_cfg_cb__coll_allgather, NULL);

    xbt_cfg_register(&_sg_cfg_set, "smpi/barrier",
                     "Which collective to use for barrier",
                     xbt_cfgelm_string, 0, 1, &_sg_cfg_cb__coll_barrier, NULL);

    xbt_cfg_register(&_sg_cfg_set, "smpi/reduce_scatter",
                     "Which collective to use for reduce_scatter",
                     xbt_cfgelm_string, 0, 1, &_sg_cfg_cb__coll_reduce_scatter, NULL);

    xbt_cfg_register(&_sg_cfg_set, "smpi/scatter",
                     "Which collective to use for scatter",
                     xbt_cfgelm_string, 0, 1, &_sg_cfg_cb__coll_scatter, NULL);

    xbt_cfg_register(&_sg_cfg_set, "smpi/allgatherv",
                     "Which collective to use for allgatherv",
                     xbt_cfgelm_string, 0, 1, &_sg_cfg_cb__coll_allgatherv, NULL);

    xbt_cfg_register(&_sg_cfg_set, "smpi/allreduce",
                     "Which collective to use for allreduce",
                     xbt_cfgelm_string, 0, 1, &_sg_cfg_cb__coll_allreduce, NULL);

    xbt_cfg_register(&_sg_cfg_set, "smpi/alltoall",
                     "Which collective to use for alltoall",
                     xbt_cfgelm_string, 0, 1, &_sg_cfg_cb__coll_alltoall, NULL);

    xbt_cfg_register(&_sg_cfg_set, "smpi/alltoallv",
                     "Which collective to use for alltoallv",
                     xbt_cfgelm_string, 0, 1, &_sg_cfg_cb__coll_alltoallv, NULL);

    xbt_cfg_register(&_sg_cfg_set, "smpi/bcast",
                     "Which collective to use for bcast",
                     xbt_cfgelm_string, 0, 1, &_sg_cfg_cb__coll_bcast, NULL);

    xbt_cfg_register(&_sg_cfg_set, "smpi/reduce",
                     "Which collective to use for reduce",
                     xbt_cfgelm_string, 0, 1, &_sg_cfg_cb__coll_reduce, NULL);
#endif // HAVE_SMPI

    xbt_cfg_register(&_sg_cfg_set, "clean_atexit",
                     "\"yes\" or \"no\". \"yes\" enables all the cleanups of SimGrid (XBT,SIMIX,MSG) to be registered with atexit. \"no\" may be useful if your code segfaults when calling the exit function.",
                     xbt_cfgelm_boolean, 1, 1, _sg_cfg_cb_clean_atexit, NULL);
    xbt_cfg_setdefault_boolean(_sg_cfg_set, "clean_atexit", "yes");

    if (!surf_path) {
      /* retrieves the current directory of the current process */
      const char *initial_path = __surf_get_initial_path();
      xbt_assert((initial_path),
                  "__surf_get_initial_path() failed! Can't resolves current Windows directory");

      surf_path = xbt_dynar_new(sizeof(char *), NULL);
      xbt_cfg_setdefault_string(_sg_cfg_set, "path", initial_path);
    }

    xbt_cfg_check(_sg_cfg_set);
    _sg_cfg_init_status = 1;

    sg_config_cmd_line(argc, argv);

    xbt_mallocator_initialization_is_done(SIMIX_context_is_parallel());

  } else {
    XBT_WARN("Call to sg_config_init() after initialization ignored");
  }
}

void sg_config_finalize(void)
{
  if (!_sg_cfg_init_status)
    return;                     /* Not initialized yet. Nothing to do */

  xbt_cfg_free(&_sg_cfg_set);
  _sg_cfg_init_status = 0;
}

/* Pick the right models for CPU, net and workstation, and call their model_init_preparse */
void surf_config_models_setup()
{
  const char *workstation_model_name;
  const char *vm_workstation_model_name;
  int workstation_id = -1;
  int vm_workstation_id = -1;
  char *network_model_name = NULL;
  char *cpu_model_name = NULL;
  int storage_id = -1;
  char *storage_model_name = NULL;

  workstation_model_name =
      xbt_cfg_get_string(_sg_cfg_set, "workstation/model");
  vm_workstation_model_name =
      xbt_cfg_get_string(_sg_cfg_set, "vm_workstation/model");
  network_model_name = xbt_cfg_get_string(_sg_cfg_set, "network/model");
  cpu_model_name = xbt_cfg_get_string(_sg_cfg_set, "cpu/model");
  storage_model_name = xbt_cfg_get_string(_sg_cfg_set, "storage/model");

  /* Check whether we use a net/cpu model differing from the default ones, in which case
   * we should switch to the "compound" workstation model to correctly dispatch stuff to
   * the right net/cpu models.
   */

  if ((!xbt_cfg_is_default_value(_sg_cfg_set, "network/model") ||
       !xbt_cfg_is_default_value(_sg_cfg_set, "cpu/model")) &&
      xbt_cfg_is_default_value(_sg_cfg_set, "workstation/model")) {
    XBT_INFO("Switching workstation model to compound since you changed the network and/or cpu model(s)");
    workstation_model_name = "compound";
    xbt_cfg_set_string(_sg_cfg_set, "workstation/model", workstation_model_name);
  }

  XBT_DEBUG("Workstation model: %s", workstation_model_name);
  workstation_id =
      find_model_description(surf_workstation_model_description,
                             workstation_model_name);
  if (!strcmp(workstation_model_name, "compound")) {
    int network_id = -1;
    int cpu_id = -1;

    xbt_assert(cpu_model_name,
                "Set a cpu model to use with the 'compound' workstation model");

    xbt_assert(network_model_name,
                "Set a network model to use with the 'compound' workstation model");

    if(surf_cpu_model_init_preparse){
      surf_cpu_model_init_preparse();
    } else {
      cpu_id =
          find_model_description(surf_cpu_model_description, cpu_model_name);
      surf_cpu_model_description[cpu_id].model_init_preparse();
    }

    network_id =
        find_model_description(surf_network_model_description,
                               network_model_name);
    surf_network_model_description[network_id].model_init_preparse();
  }

  XBT_DEBUG("Call workstation_model_init");
  surf_workstation_model_description[workstation_id].model_init_preparse();

  XBT_DEBUG("Call vm_workstation_model_init");
  vm_workstation_id = find_model_description(surf_vm_workstation_model_description,
                                          vm_workstation_model_name);
  surf_vm_workstation_model_description[vm_workstation_id].model_init_preparse();

  XBT_DEBUG("Call storage_model_init");
  storage_id = find_model_description(surf_storage_model_description, storage_model_name);
  surf_storage_model_description[storage_id].model_init_preparse();

}

int sg_cfg_is_default_value(const char *name)
{
  return xbt_cfg_is_default_value(_sg_cfg_set, name);
}

int sg_cfg_get_int(const char* name)
{
  return xbt_cfg_get_int(_sg_cfg_set, name);
}

double sg_cfg_get_double(const char* name)
{
  return xbt_cfg_get_double(_sg_cfg_set, name);
}

char* sg_cfg_get_string(const char* name)
{
  return xbt_cfg_get_string(_sg_cfg_set, name);
}

int sg_cfg_get_boolean(const char* name)
{
  return xbt_cfg_get_boolean(_sg_cfg_set, name);
}

void sg_cfg_get_peer(const char *name, char **peer, int *port)
{
  xbt_cfg_get_peer(_sg_cfg_set, name, peer, port);
}

xbt_dynar_t sg_cfg_get_dynar(const char* name)
{
  return xbt_cfg_get_dynar(_sg_cfg_set, name);
}
