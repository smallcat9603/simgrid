/* Copyright (c) 2007-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <stdio.h>
#include "msg/msg.h"            /* Yeah! If you want to use msg, you need to include msg/msg.h */
#include "xbt/sysdep.h"         /* calloc, printf */

/* Create a log channel to have nice outputs. */
#include "xbt/log.h"
#include "xbt/asserts.h"
XBT_LOG_NEW_DEFAULT_CATEGORY(msg_test,
                             "Messages specific for this msg example");

/** @addtogroup MSG_examples
 * 
 * - <b>priority/priority.c</b>: Demonstrates the use of @ref
 *   MSG_task_set_priority to change the computation priority of a
 *   given task.
 *
 */

static int test(int argc, char *argv[])
{
  double computation_amount = 0.0;
  double priority = 1.0;
  msg_task_t task = NULL;

  _XBT_GNUC_UNUSED int res = sscanf(argv[1], "%lg", &computation_amount);
  xbt_assert(res, "Invalid argument %s\n", argv[1]);
  res = sscanf(argv[2], "%lg", &priority);
  xbt_assert(res, "Invalid argument %s\n", argv[2]);

  XBT_INFO("Hello! Running a task of size %g with priority %g",
        computation_amount, priority);
  task = MSG_task_create("Task", computation_amount, 0.0, NULL);
  MSG_task_set_priority(task, priority);

  MSG_task_execute(task);
  MSG_task_destroy(task);

  XBT_INFO("Goodbye now!");
  return 0;
}

static msg_error_t test_all(const char *platform_file,
                            const char *application_file)
{
  msg_error_t res = MSG_OK;

  {                             /*  Simulation setting */
    MSG_create_environment(platform_file);
  }
  {                             /*   Application deployment */
    MSG_function_register("test", test);
    MSG_launch_application(application_file);
  }
  res = MSG_main();

  XBT_INFO("Simulation time %g", MSG_get_clock());
  return res;
}

int main(int argc, char *argv[])
{
  msg_error_t res = MSG_OK;

#ifdef _MSC_VER
  unsigned int prev_exponent_format =
      _set_output_format(_TWO_DIGIT_EXPONENT);
#endif


  MSG_init(&argc, argv);
  if (argc < 3) {
    printf("Usage: %s platform_file deployment_file\n", argv[0]);
    printf("example: %s msg_platform.xml msg_deployment.xml\n", argv[0]);
    exit(1);
  }
  res = test_all(argv[1], argv[2]);

#ifdef _MSC_VER
  _set_output_format(prev_exponent_format);
#endif

  if (res == MSG_OK)
    return 0;
  else
    return 1;
}
