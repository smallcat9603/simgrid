/* Copyright (c) 2008-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt/config.hpp"
#include "xbt/log.h"
#include <xbt/sysdep.h>

#include "src/mc/mc_replay.hpp"
#include <mc/mc.h>

#include <simgrid/sg_config.hpp>

#if SIMGRID_HAVE_MC
#include "src/mc/mc_private.hpp"
#include "src/mc/mc_safety.hpp"
#endif

#include "src/mc/mc_record.hpp"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(mc_config, mc, "Configuration of the Model Checker");

#if SIMGRID_HAVE_MC
namespace simgrid {
namespace mc {
/* Configuration support */
simgrid::mc::ReductionMode reduction_mode = simgrid::mc::ReductionMode::unset;
}
}
#endif

#if !SIMGRID_HAVE_MC
#define _sg_do_model_check 0
#endif

static void _mc_cfg_cb_check(const char* spec, bool more_check = true)
{
  if (_sg_cfg_init_status && not _sg_do_model_check && more_check)
    xbt_die("You are specifying a %s after the initialization (through MSG_config?), but the program was not run under "
            "the model-checker (with simgrid-mc)). This won't work, sorry.",
            spec);
}

/* Replay (this part is enabled even if MC it disabled) */
simgrid::config::Flag<std::string> _sg_mc_record_path{
    "model-check/replay", "Model-check path to replay (as reported by SimGrid when a violation is reported)", ""};

simgrid::config::Flag<bool> _sg_mc_timeout{
    "model-check/timeout", "Whether to enable timeouts for wait requests", false,
    [](bool) { _mc_cfg_cb_check("value to enable/disable timeout for wait requests", MC_record_path.empty()); }};

#if SIMGRID_HAVE_MC
int _sg_do_model_check = 0;
int _sg_mc_max_visited_states = 0;

simgrid::config::Flag<bool> _sg_do_model_check_record{"model-check/record", "Record the model-checking paths", false};

simgrid::config::Flag<int> _sg_mc_checkpoint{
    "model-check/checkpoint", "Specify the amount of steps between checkpoints during stateful model-checking "
                              "(default: 0 => stateless verification). If value=1, one checkpoint is saved for each "
                              "step => faster verification, but huge memory consumption; higher values are good "
                              "compromises between speed and memory consumption.",
    0, [](int) { _mc_cfg_cb_check("checkpointing value"); }};

simgrid::config::Flag<bool> _sg_mc_sparse_checkpoint{"model-check/sparse-checkpoint", "Use sparse per-page snapshots.",
                                                     false, [](bool) { _mc_cfg_cb_check("checkpointing value"); }};

simgrid::config::Flag<bool> _sg_mc_ksm{"model-check/ksm", "Kernel same-page merging", false,
                                       [](bool) { _mc_cfg_cb_check("KSM value"); }};

simgrid::config::Flag<std::string> _sg_mc_property_file{
    "model-check/property", "Name of the file containing the property, as formatted by the ltl2ba program.", "",
    [](const std::string&) { _mc_cfg_cb_check("property"); }};

simgrid::config::Flag<bool> _sg_mc_comms_determinism{
    "model-check/communications-determinism",
    {"model-check/communications_determinism"},
    "Whether to enable the detection of communication determinism",
    false,
    [](bool) {
      _mc_cfg_cb_check("value to enable/disable the detection of determinism in the communications schemes");
    }};

simgrid::config::Flag<bool> _sg_mc_send_determinism{
    "model-check/send-determinism",
    {"model-check/send_determinism"},
    "Enable/disable the detection of send-determinism in the communications schemes",
    false,
    [](bool) {
      _mc_cfg_cb_check("value to enable/disable the detection of send-determinism in the communications schemes");
    }};

static simgrid::config::Flag<std::string> _sg_mc_reduce{
    "model-check/reduction", "Specify the kind of exploration reduction (either none or DPOR)", "dpor",
    [](const std::string& value) {
      _mc_cfg_cb_check("reduction strategy");

      if (value == "none")
        simgrid::mc::reduction_mode = simgrid::mc::ReductionMode::none;
      else if (value == "dpor")
        simgrid::mc::reduction_mode = simgrid::mc::ReductionMode::dpor;
      else
        xbt_die("configuration option model-check/reduction can only take 'none' or 'dpor' as a value");
    }};

simgrid::config::Flag<bool> _sg_mc_hash{
    "model-check/hash", "Whether to enable state hash for state comparison (experimental)", false,
    [](bool) { _mc_cfg_cb_check("value to enable/disable the use of global hash to speedup state comparaison"); }};

simgrid::config::Flag<bool> _sg_mc_snapshot_fds{
    "model-check/snapshot-fds",
    {"model-check/snapshot_fds"},
    "Whether file descriptors must be snapshoted (currently unusable)",
    false,
    [](bool) { _mc_cfg_cb_check("value to enable/disable the use of FD snapshotting"); }};

simgrid::config::Flag<int> _sg_mc_max_depth{"model-check/max-depth",
                                            {"model-check/max_depth"},
                                            "Maximal exploration depth (default: 1000)",
                                            1000,
                                            [](int) { _mc_cfg_cb_check("max depth value"); }};

static simgrid::config::Flag<int> _sg_mc_max_visited_states__{
    "model-check/visited", "Specify the number of visited state stored for state comparison reduction. If value=5, the "
                           "last 5 visited states are stored. If value=0 (the default), all states are stored.",
    0, [](int value) {
      _mc_cfg_cb_check("number of stored visited states");
      _sg_mc_max_visited_states = value;
    }};

simgrid::config::Flag<std::string> _sg_mc_dot_output_file{
    "model-check/dot-output",
    {"model-check/dot_output"},
    "Name of dot output file corresponding to graph state",
    "",
    [](const std::string&) { _mc_cfg_cb_check("file name for a dot output of graph state"); }};

simgrid::config::Flag<bool> _sg_mc_termination{
    "model-check/termination", "Whether to enable non progressive cycle detection", false,
    [](bool) { _mc_cfg_cb_check("value to enable/disable the detection of non progressive cycles"); }};

#endif
