/* Copyright (c) 2007-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include<stdio.h>

#include "msg/msg.h"            /* Yeah! If you want to use msg, you need to include msg/msg.h */
#include "xbt/sysdep.h"         /* calloc */

/* Create a log channel to have nice outputs. */
#include "xbt/log.h"
#include "xbt/asserts.h"

/** @addtogroup MSG_examples
 *
 *  - <b>sendrecv/sendrecv.c: Ping-pong example</b>. It's hard to
 *    think of a simpler example. The tesh files laying in the
 *    directory are instructive concerning the way to pass options to the simulators (as described in \ref options).
 */

XBT_LOG_NEW_DEFAULT_CATEGORY(msg_test,
                             "Messages specific for this msg example");

int sender(int argc, char *argv[]);
int receiver(int argc, char *argv[]);

msg_error_t test_all(const char *platform_file,
                     const char *application_file);

double task_comm_size_lat = 1;
double task_comm_size_bw = 10e8;

/** Emitter function  */
int sender(int argc, char *argv[])
{
  msg_host_t host = NULL;
  double time;
  msg_task_t task_la = NULL;
  msg_task_t task_bw = NULL;
  char sprintf_buffer_la[64];
  char sprintf_buffer_bw[64];

  XBT_INFO("sender");

  /*host = xbt_new0(msg_host_t,1); */

  XBT_INFO("host = %s", argv[1]);

  host = MSG_get_host_by_name(argv[1]);

  if (host == NULL) {
    XBT_INFO("Unknown host %s. Stopping Now! ", argv[1]);
    abort();
  }

  /* Latency */
  time = MSG_get_clock();
  sprintf(sprintf_buffer_la, "latency task");
  task_la =
      MSG_task_create(sprintf_buffer_la, 0.0, task_comm_size_lat, NULL);
  task_la->data = xbt_new(double, 1);
  *(double *) task_la->data = time;
  XBT_INFO("task_la->data = %e", *((double *) task_la->data));
  MSG_task_send(task_la, argv[1]);

  /* Bandwidth */
  time = MSG_get_clock();
  sprintf(sprintf_buffer_bw, "bandwidth task");
  task_bw =
      MSG_task_create(sprintf_buffer_bw, 0.0, task_comm_size_bw, NULL);
  task_bw->data = xbt_new(double, 1);
  *(double *) task_bw->data = time;
  XBT_INFO("task_bw->data = %e", *((double *) task_bw->data));
  MSG_task_send(task_bw, argv[1]);

  return 0;
}                               /* end_of_client */

/** Receiver function  */
int receiver(int argc, char *argv[])
{
  double time, time1, sender_time;
  msg_task_t task_la = NULL;
  msg_task_t task_bw = NULL;
  int a;
  double communication_time = 0;

  XBT_INFO("receiver");

  /* Get Latency */
  a = MSG_task_receive(&task_la,MSG_host_get_name(MSG_host_self()));
  if (a == MSG_OK) {
    time1 = MSG_get_clock();
    sender_time = *((double *) (task_la->data));
    time = sender_time;
    communication_time = time1 - time;
    XBT_INFO("Task received : %s", task_la->name);
    xbt_free(task_la->data);
    MSG_task_destroy(task_la);
    XBT_INFO("Communic. time %e", communication_time);
    XBT_INFO("--- la %f ----", communication_time);
  } else {
    xbt_die("Unexpected behavior");
  }

  /* Get Bandwidth */
  a = MSG_task_receive(&task_bw,MSG_host_get_name(MSG_host_self()));
  if (a == MSG_OK) {
    time1 = MSG_get_clock();
    sender_time = *((double *) (task_bw->data));
    time = sender_time;
    communication_time = time1 - time;
    XBT_INFO("Task received : %s", task_bw->name);
    xbt_free(task_bw->data);
    MSG_task_destroy(task_bw);
    XBT_INFO("Communic. time %e", communication_time);
    XBT_INFO("--- bw %f ----", task_comm_size_bw / communication_time);
  } else {
    xbt_die("Unexpected behavior");
  }


  return 0;
}                               /* end_of_receiver */


/** Test function */
msg_error_t test_all(const char *platform_file,
                     const char *application_file)
{
  msg_error_t res = MSG_OK;

  XBT_INFO("test_all");

  /*  Simulation setting */
  MSG_create_environment(platform_file);

  /*   Application deployment */
  MSG_function_register("sender", sender);
  MSG_function_register("receiver", receiver);

  MSG_launch_application(application_file);

  res = MSG_main();

  return res;
}                               /* end_of_test_all */


/** Main function */
int main(int argc, char *argv[])
{
  msg_error_t res = MSG_OK;

#ifdef _MSC_VER
  unsigned int prev_exponent_format =
      _set_output_format(_TWO_DIGIT_EXPONENT);
#endif

  MSG_init(&argc, argv);

  if (argc != 3) {
    XBT_CRITICAL("Usage: %s platform_file deployment_file <model>\n",
              argv[0]);
    XBT_CRITICAL
        ("example: %s msg_platform.xml msg_deployment.xml KCCFLN05_Vegas\n",
         argv[0]);
    exit(1);
  }

  /* Options for the workstation/model:

     KCCFLN05              => for maxmin
     KCCFLN05_proportional => for proportional (Vegas)
     KCCFLN05_Vegas        => for TCP Vegas
     KCCFLN05_Reno         => for TCP Reno
   */
  //MSG_config("workstation/model", argv[3]);

  res = test_all(argv[1], argv[2]);

  XBT_INFO("Total simulation time: %e", MSG_get_clock());

#ifdef _MSC_VER
  _set_output_format(prev_exponent_format);
#endif

  if (res == MSG_OK)
    return 0;
  else
    return 1;
}                               /* end_of_main */
