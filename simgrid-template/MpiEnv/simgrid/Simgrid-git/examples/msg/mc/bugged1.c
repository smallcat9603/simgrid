/* Copyright (c) 2010-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/******************** Non-deterministic message ordering  *********************/
/* Server assumes a fixed order in the reception of messages from its clients */
/* which is incorrect because the message ordering is non-deterministic       */
/******************************************************************************/

#include <msg/msg.h>
#include <simgrid/modelchecker.h>

#define N 3

XBT_LOG_NEW_DEFAULT_CATEGORY(example, "this example");

int server(int argc, char *argv[]);
int client(int argc, char *argv[]);

int server(int argc, char *argv[])
{
  msg_task_t task = NULL;
  int count = 0;
  while (count < N) {
    if (task) {
      MSG_task_destroy(task);
      task = NULL;
    }
    MSG_task_receive(&task, "mymailbox");
    count++;
  }
  MC_assert(atoi(MSG_task_get_name(task)) == 3);

  XBT_INFO("OK");
  return 0;
}

int client(int argc, char *argv[])
{

  msg_task_t task =
      MSG_task_create(argv[1], 0 /*comp cost */ , 10000 /*comm size */ ,
                      NULL /*arbitrary data */ );

  MSG_task_send(task, "mymailbox");

  XBT_INFO("Sent!");
  return 0;
}

int main(int argc, char *argv[])
{

  MSG_init(&argc, argv);

  MSG_create_environment("platform.xml");

  MSG_function_register("server", server);

  MSG_function_register("client", client);

  MSG_launch_application("deploy_bugged1.xml");

  MSG_main();

  return 0;

}
