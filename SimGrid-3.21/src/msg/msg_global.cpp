/* Copyright (c) 2004-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/s4u/Engine.hpp"
#include "simgrid/s4u/Host.hpp"

#include "mc/mc.h"
#include "src/instr/instr_private.hpp"
#include "src/msg/msg_private.hpp"
#include <xbt/config.hpp>

XBT_LOG_NEW_CATEGORY(msg, "All MSG categories");
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(msg_kernel, msg, "Logging specific to MSG (kernel)");

MSG_Global_t msg_global = nullptr;
static void MSG_exit();

/********************************* MSG **************************************/

/**
 * @ingroup msg_simulation
 * @brief Initialize MSG with less verifications
 * You should use the MSG_init() function instead. Failing to do so may turn into PEBKAC some day. You've been warned.
 */
void MSG_init_nocheck(int *argc, char **argv) {

  TRACE_global_init();

  if (not msg_global) {

    msg_global = new s_MSG_Global_t();

    msg_global->debug_multiple_use = false;
    simgrid::config::bind_flag(msg_global->debug_multiple_use, "msg/debug-multiple-use",
                               "Print backtraces of both processes when there is a conflict of multiple use of a task");

    SIMIX_global_init(argc, argv);

    msg_global->sent_msg = 0;
    msg_global->task_copy_callback = nullptr;
    msg_global->process_data_cleanup = nullptr;

    SIMIX_function_register_process_create(MSG_process_create_from_SIMIX);
    SIMIX_function_register_process_cleanup(MSG_process_cleanup_from_SIMIX);
  }

  if(MC_is_active()){
    /* Ignore total amount of messages sent during the simulation for heap comparison */
    MC_ignore_heap(&(msg_global->sent_msg), sizeof(msg_global->sent_msg));
  }

  if (simgrid::config::get_value<bool>("clean-atexit"))
    atexit(MSG_exit);
}

void MSG_config(const char *key, const char *value){
  xbt_assert(msg_global,"ERROR: Please call MSG_init() before using MSG_config()");
  simgrid::config::set_as_string(key, value);
}

static void MSG_exit() {
  delete msg_global;
  msg_global = nullptr;
}

unsigned long int MSG_get_sent_msg()
{
  return msg_global->sent_msg;
}

/** @brief register functions bypassing the parser */
void MSG_set_function(const char* host_id, const char* function_name, xbt_dynar_t arguments)
{
  SIMIX_process_set_function(host_id, function_name, arguments, -1, -1);
}
