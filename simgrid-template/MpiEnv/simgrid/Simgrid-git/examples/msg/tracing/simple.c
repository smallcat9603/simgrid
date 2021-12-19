/* Copyright (c) 2010, 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <stdio.h>
#include "msg/msg.h"
#include "xbt/sysdep.h"         /* calloc, printf */

/** @addtogroup MSG_examples
 * 
 * @section MSG_ex_tracing Tracing and vizualization features
 * 
 * - <b>tracing/simple.c</b> very simple program where each process creates, executes and
 *   destroy a task. You might want to run this program with the following parameters:
 *   --cfg=tracing/uncategorized:yes
 *   (See \ref tracing_tracing_options for details)
 */

/* Create a log channel to have nice outputs. */
#include "xbt/log.h"
#include "xbt/asserts.h"
XBT_LOG_NEW_DEFAULT_CATEGORY(msg_test,
                             "Messages specific for this msg example");

int simple_func(int argc, char *argv[]);

/** Emitter function  */
int simple_func(int argc, char *argv[])
{
  msg_task_t task = MSG_task_create("task", 100, 0, NULL);
  MSG_task_execute (task);
  MSG_task_destroy (task);
  return 0;
}

/** Main function */
int main(int argc, char *argv[])
{
  MSG_init(&argc, argv);
  if (argc < 3) {
    printf("Usage: %s platform_file deployment_file\n", argv[0]);
    exit(1);
  }

  char *platform_file = argv[1];
  char *deployment_file = argv[2];
  MSG_create_environment(platform_file);

  MSG_function_register("master", simple_func);
  MSG_function_register("slave", simple_func);
  MSG_launch_application(deployment_file);

  MSG_main();
  return 0;
}
