/* Copyright (c) 2009-2010, 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <stdio.h>
#include <stdlib.h>
#include "msg/msg.h"
#include "xbt/log.h"
#include "xbt/asserts.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(test_trace_integration,
                             "Messages specific for this msg example");

int test_trace(int argc, char *argv[]);

/** test the trace integration cpu model */
int test_trace(int argc, char *argv[])
{
  msg_task_t task;
  double task_comp_size = 2800;
  double task_prio = 1.0;

  if (argc != 3) {
    printf
        ("Wrong number of arguments!\nUsage:\n\t1) task computational size in FLOPS\n\t2 task priority\n");
    exit(1);
  }

  task_comp_size = atof(argv[1]);
  task_prio = atof(argv[2]);

  XBT_INFO("Testing the trace integration cpu model: CpuTI");
  XBT_INFO("Task size: %f", task_comp_size);
  XBT_INFO("Task prio: %f", task_prio);

  /* Create and execute a single task. */
  task = MSG_task_create("proc 0", task_comp_size, 0, NULL);
  MSG_task_set_priority(task, task_prio);
  MSG_task_execute(task);
  MSG_task_destroy(task);

  XBT_INFO("Test finished");

  return 0;
}

/** Main function */
int main(int argc, char *argv[])
{
  msg_error_t res = MSG_OK;

  /* Verify if the platform xml file was passed by command line. */
  MSG_init(&argc, argv);
  if (argc < 2) {
    printf("Usage: %s test_trace_integration_model.xml\n", argv[0]);
    exit(1);
  }

  /* Register SimGrid process function. */
  MSG_function_register("test_trace", test_trace);
  /* Use the same file for platform and deployment. */
  MSG_create_environment(argv[1]);
  MSG_launch_application(argv[2]);
  /* Run the example. */
  res = MSG_main();

  if (res == MSG_OK)
    return 0;
  else
    return 1;
}
