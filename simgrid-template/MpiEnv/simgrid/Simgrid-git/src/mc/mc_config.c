/* Copyright (c) 2008-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <strings.h>

#include <xbt/log.h>
#include <xbt/config.h>

#include <mc/mc.h>

#include <simgrid/sg_config.h>

#include "mc_private.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(mc_config, mc,
                                "Configuration of MC");

/* Configuration support */
e_mc_reduce_t mc_reduce_kind = e_mc_reduce_unset;

int _sg_do_model_check = 0;
int _sg_mc_checkpoint = 0;
int _sg_mc_sparse_checkpoint = 0;
int _sg_mc_soft_dirty = 0;
char *_sg_mc_property_file = NULL;
int _sg_mc_timeout = 0;
int _sg_mc_hash = 0;
int _sg_mc_max_depth = 1000;
int _sg_mc_visited = 0;
char *_sg_mc_dot_output_file = NULL;
int _sg_mc_comms_determinism = 0;
int _sg_mc_send_determinism = 0;
int _sg_mc_safety = 0;
int _sg_mc_liveness = 0;


void _mc_cfg_cb_reduce(const char *name, int pos)
{
  if (_sg_cfg_init_status && !_sg_do_model_check) {
    xbt_die
        ("You are specifying a reduction strategy after the initialization (through MSG_config?), but model-checking was not activated at config time (through --cfg=model-check:1). This won't work, sorry.");
  }
  char *val = xbt_cfg_get_string(_sg_cfg_set, name);
  if (!strcasecmp(val, "none")) {
    mc_reduce_kind = e_mc_reduce_none;
  } else if (!strcasecmp(val, "dpor")) {
    mc_reduce_kind = e_mc_reduce_dpor;
  } else {
    xbt_die("configuration option %s can only take 'none' or 'dpor' as a value",
            name);
  }
}

void _mc_cfg_cb_checkpoint(const char *name, int pos)
{
  if (_sg_cfg_init_status && !_sg_do_model_check) {
    xbt_die
        ("You are specifying a checkpointing value after the initialization (through MSG_config?), but model-checking was not activated at config time (through --cfg=model-check:1). This won't work, sorry.");
  }
  _sg_mc_checkpoint = xbt_cfg_get_int(_sg_cfg_set, name);
}

void _mc_cfg_cb_sparse_checkpoint(const char *name, int pos) {
  if (_sg_cfg_init_status && !_sg_do_model_check) {
    xbt_die("You are specifying a checkpointing value after the initialization (through MSG_config?), but model-checking was not activated at config time (through --cfg=model-check:1). This won't work, sorry.");
  }
  _sg_mc_sparse_checkpoint = xbt_cfg_get_boolean(_sg_cfg_set, name);
}

void _mc_cfg_cb_soft_dirty(const char *name, int pos) {
  if (_sg_cfg_init_status && !_sg_do_model_check) {
    xbt_die("You are specifying a soft dirty value after the initialization (through MSG_config?), but model-checking was not activated at config time (through --cfg=model-check:1). This won't work, sorry.");
  }
  _sg_mc_soft_dirty = xbt_cfg_get_boolean(_sg_cfg_set, name);
}

void _mc_cfg_cb_property(const char *name, int pos)
{
  if (_sg_cfg_init_status && !_sg_do_model_check) {
    xbt_die
        ("You are specifying a property after the initialization (through MSG_config?), but model-checking was not activated at config time (through --cfg=model-check:1). This won't work, sorry.");
  }
  _sg_mc_property_file = xbt_cfg_get_string(_sg_cfg_set, name);
}

void _mc_cfg_cb_timeout(const char *name, int pos)
{
  if (_sg_cfg_init_status && !_sg_do_model_check) {
    xbt_die
        ("You are specifying a value to enable/disable timeout for wait requests after the initialization (through MSG_config?), but model-checking was not activated at config time (through --cfg=model-check:1). This won't work, sorry.");
  }
  _sg_mc_timeout = xbt_cfg_get_boolean(_sg_cfg_set, name);
}

void _mc_cfg_cb_hash(const char *name, int pos)
{
  if (_sg_cfg_init_status && !_sg_do_model_check) {
    xbt_die
        ("You are specifying a value to enable/disable the use of global hash to speedup state comparaison, but model-checking was not activated at config time (through --cfg=model-check:1). This won't work, sorry.");
  }
  _sg_mc_hash = xbt_cfg_get_boolean(_sg_cfg_set, name);
}

void _mc_cfg_cb_max_depth(const char *name, int pos)
{
  if (_sg_cfg_init_status && !_sg_do_model_check) {
    xbt_die
        ("You are specifying a max depth value after the initialization (through MSG_config?), but model-checking was not activated at config time (through --cfg=model-check:1). This won't work, sorry.");
  }
  _sg_mc_max_depth = xbt_cfg_get_int(_sg_cfg_set, name);
}

void _mc_cfg_cb_visited(const char *name, int pos)
{
  if (_sg_cfg_init_status && !_sg_do_model_check) {
    xbt_die
        ("You are specifying a number of stored visited states after the initialization (through MSG_config?), but model-checking was not activated at config time (through --cfg=model-check:1). This won't work, sorry.");
  }
  _sg_mc_visited = xbt_cfg_get_int(_sg_cfg_set, name);
}

void _mc_cfg_cb_dot_output(const char *name, int pos)
{
  if (_sg_cfg_init_status && !_sg_do_model_check) {
    xbt_die
        ("You are specifying a file name for a dot output of graph state after the initialization (through MSG_config?), but model-checking was not activated at config time (through --cfg=model-check:1). This won't work, sorry.");
  }
  _sg_mc_dot_output_file = xbt_cfg_get_string(_sg_cfg_set, name);
}

void _mc_cfg_cb_comms_determinism(const char *name, int pos)
{
  if (_sg_cfg_init_status && !_sg_do_model_check) {
    xbt_die
        ("You are specifying a value to enable/disable the detection of determinism in the communications schemes after the initialization (through MSG_config?), but model-checking was not activated at config time (through --cfg=model-check:1). This won't work, sorry.");
  }
  _sg_mc_comms_determinism = xbt_cfg_get_boolean(_sg_cfg_set, name);
  mc_reduce_kind = e_mc_reduce_none;
}

void _mc_cfg_cb_send_determinism(const char *name, int pos)
{
  if (_sg_cfg_init_status && !_sg_do_model_check) {
    xbt_die
        ("You are specifying a value to enable/disable the detection of send-determinism in the communications schemes after the initialization (through MSG_config?), but model-checking was not activated at config time (through --cfg=model-check:1). This won't work, sorry.");
  }
  _sg_mc_send_determinism = xbt_cfg_get_boolean(_sg_cfg_set, name);
  mc_reduce_kind = e_mc_reduce_none;
}
