/* Copyright (c) 2010-2014. The SimGrid Team.
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

int master(int argc, char *argv[]);
int slave(int argc, char *argv[]);

/** Emitter function  */
int master(int argc, char *argv[])
{
  double task_comp_size = 5E7;
  double task_comm_size = 1E6;

  char mailbox[256];
  msg_task_t task = NULL;
  msg_host_t jupiter = MSG_get_host_by_name("Jupiter");
  sprintf(mailbox, "jupi");

  task = MSG_task_create("task on", task_comp_size, task_comm_size, NULL);
  XBT_INFO("Sending \"%s\"", task->name);
  if (MSG_task_send_with_timeout(task, mailbox, 1) != MSG_OK)
    MSG_task_destroy(task);

  MSG_process_sleep(1);
  MSG_host_off(jupiter);

  task = MSG_task_create("task off", task_comp_size, task_comm_size, NULL);
  XBT_INFO("Sending \"%s\"", task->name);
  if (MSG_task_send_with_timeout(task, mailbox, 1) != MSG_OK)
    MSG_task_destroy(task);

  MSG_host_on(jupiter);
  xbt_swag_t jupi_processes = MSG_host_get_process_list(jupiter);
  void *process;
  xbt_swag_foreach(process, jupi_processes) {
    MSG_process_kill(process);
  }

  task = MSG_task_create("task on without proc", task_comp_size, task_comm_size, NULL);
  XBT_INFO("Sending \"%s\"", task->name);
  if (MSG_task_send_with_timeout(task, mailbox, 1) != MSG_OK)
    MSG_task_destroy(task);

  char **argvF = xbt_new(char*, 2);
  argvF[0] = xbt_strdup("slave");
  MSG_process_create_with_arguments("slave", slave, NULL, MSG_get_host_by_name("Jupiter"), 1, argvF);

  task = MSG_task_create("task on with proc", task_comp_size, task_comm_size, NULL);
  XBT_INFO("Sending \"%s\"", task->name);
  if (MSG_task_send_with_timeout(task, mailbox, 1) != MSG_OK)
    MSG_task_destroy(task);

  task = MSG_task_create("finalize", 0, 0, 0);
  XBT_INFO("Sending \"%s\"", task->name);
  if (MSG_task_send_with_timeout(task, mailbox, 1) != MSG_OK)
    MSG_task_destroy(task);

  XBT_INFO("Goodbye now!");
  return 0;
}                               /* end_of_master */

/** Receiver function  */
int slave(int argc, char *argv[])
{
  msg_task_t task = NULL;
  _XBT_GNUC_UNUSED int res;
  int id = -1;
  char mailbox[80];

  sprintf(mailbox, "jupi");

  while (1) {
    res = MSG_task_receive(&(task), mailbox);
    xbt_assert(res == MSG_OK, "MSG_task_get failed");

    if (!strcmp(MSG_task_get_name(task), "finalize")) {
      MSG_task_destroy(task);
      break;
    }
    MSG_task_execute(task);
    XBT_INFO("Task \"%s\" done", MSG_task_get_name(task));

    MSG_task_destroy(task);
    task = NULL;
    id--;
  }
  XBT_INFO("I'm done. See you!");
  return 0;
}                               /* end_of_slave */

/** Main function */
int main(int argc, char *argv[])
{
  msg_error_t res;
  const char *platform_file;
  const char *application_file;

  MSG_init(&argc, argv);
  if (argc != 3) {
    printf("Usage: %s platform_file deployment_file\n", argv[0]);
    printf("example: %s msg_platform.xml msg_deployment.xml\n", argv[0]);
    exit(1);
  }
  platform_file = argv[1];
  application_file = argv[2];

  /* MSG_config("workstation/model","KCCFLN05"); */
  {                             /*  Simulation setting */
    MSG_create_environment(platform_file);
  }
  {                             /*   Application deployment */
    MSG_function_register("master", master);
    MSG_function_register("slave", slave);

    MSG_launch_application(application_file);
  }
  res = MSG_main();

  XBT_INFO("Simulation time %g", MSG_get_clock());

  if (res == MSG_OK)
    return 0;
  else
    return 1;
}                               /* end_of_main */
