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

/** @addtogroup MSG_examples
 * 
 * - <b>msg/icomms/peer2.c</b>: demonstrates the @ref MSG_comm_waitall function
 */

int sender(int argc, char *argv[]);
int receiver(int argc, char *argv[]);

msg_error_t test_all(const char *platform_file,
                     const char *application_file);

/** Sender function  */
int sender(int argc, char *argv[])
{
  long number_of_tasks = atol(argv[1]);
  double task_comp_size = atof(argv[2]);
  double task_comm_size = atof(argv[3]);
  long receivers_count = atol(argv[4]);

  msg_comm_t *comm = xbt_new(msg_comm_t, number_of_tasks + receivers_count);
  int i;
  msg_task_t task = NULL;
  for (i = 0; i < number_of_tasks; i++) {
    char mailbox[256];
    char sprintf_buffer[256];
    sprintf(mailbox, "receiver-%ld", i % receivers_count);
    sprintf(sprintf_buffer, "Task_%d", i);
    task =
        MSG_task_create(sprintf_buffer, task_comp_size, task_comm_size,
                        NULL);
    comm[i] = MSG_task_isend(task, mailbox);
    XBT_INFO("Send to receiver-%ld Task_%d", i % receivers_count, i);
  }
  for (i = 0; i < receivers_count; i++) {
    char mailbox[80];
    sprintf(mailbox, "receiver-%ld", i % receivers_count);
    task = MSG_task_create("finalize", 0, 0, 0);
    comm[i + number_of_tasks] = MSG_task_isend(task, mailbox);
    XBT_INFO("Send to receiver-%ld finalize", i % receivers_count);

  }
  /* Here we are waiting for the completion of all communications */
  MSG_comm_waitall(comm, (number_of_tasks + receivers_count), -1);
  for (i = 0; i < number_of_tasks + receivers_count; i++)
    MSG_comm_destroy(comm[i]);

  XBT_INFO("Goodbye now!");
  xbt_free(comm);
  return 0;
}                               /* end_of_sender */

/** Receiver function  */
int receiver(int argc, char *argv[])
{
  msg_task_t task = NULL;
  _XBT_GNUC_UNUSED msg_error_t res;
  int id = -1;
  char mailbox[80];
  msg_comm_t res_irecv;
  _XBT_GNUC_UNUSED int read;
  read = sscanf(argv[1], "%d", &id);
  xbt_assert(read, "Invalid argument %s\n", argv[1]);
  MSG_process_sleep(10);
  sprintf(mailbox, "receiver-%d", id);
  while (1) {
    res_irecv = MSG_task_irecv(&(task), mailbox);
    XBT_INFO("Wait to receive a task");
    res = MSG_comm_wait(res_irecv, -1);
    MSG_comm_destroy(res_irecv);
    xbt_assert(res == MSG_OK, "MSG_task_get failed");
    XBT_INFO("Received \"%s\"", MSG_task_get_name(task));
    if (!strcmp(MSG_task_get_name(task), "finalize")) {
      MSG_task_destroy(task);
      break;
    }

    XBT_INFO("Processing \"%s\"", MSG_task_get_name(task));
    MSG_task_execute(task);
    XBT_INFO("\"%s\" done", MSG_task_get_name(task));
    MSG_task_destroy(task);
    task = NULL;
  }
  XBT_INFO("I'm done. See you!");
  return 0;
}                               /* end_of_receiver */

/** Test function */
msg_error_t test_all(const char *platform_file,
                     const char *application_file)
{
  msg_error_t res = MSG_OK;

  /* MSG_config("workstation/model","KCCFLN05"); */
  {                             /*  Simulation setting */
    MSG_create_environment(platform_file);
  }
  {                             /*   Application deployment */
    MSG_function_register("sender", sender);
    MSG_function_register("receiver", receiver);
    MSG_launch_application(application_file);
  }
  res = MSG_main();

  XBT_INFO("Simulation time %g", MSG_get_clock());
  return res;
}                               /* end_of_test_all */


/** Main function */
int main(int argc, char *argv[])
{
  msg_error_t res = MSG_OK;

  MSG_init(&argc, argv);
  if (argc < 3) {
    printf("Usage: %s platform_file deployment_file\n", argv[0]);
    printf("example: %s msg_platform.xml msg_deployment.xml\n", argv[0]);
    exit(1);
  }
  res = test_all(argv[1], argv[2]);

  if (res == MSG_OK)
    return 0;
  else
    return 1;
}                               /* end_of_main */
