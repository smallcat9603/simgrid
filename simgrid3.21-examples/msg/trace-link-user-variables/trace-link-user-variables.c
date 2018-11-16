/* Copyright (c) 2012-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <simgrid/msg.h>

//dump function to create and execute a task
static void create_and_execute_task (void)
{
  msg_task_t task = MSG_task_create("task", 1000000, 0, NULL);
  MSG_task_execute (task);
  MSG_task_destroy (task);
}

static int trace_fun(int argc, char *argv[])
{
  //set initial values for the link user variables this example only shows for links identified by "6" and "3" in the
  //platform file

  //Set the Link_Capacity variable
  TRACE_link_variable_set("6", "Link_Capacity", 12.34);
  TRACE_link_variable_set("3", "Link_Capacity", 56.78);

  //Set the Link_Utilization variable
  TRACE_link_variable_set("3", "Link_Utilization", 1.2);
  TRACE_link_variable_set("6", "Link_Utilization", 3.4);

  //run the simulation, update my variables accordingly
  for (int i = 0; i < 10; i++) {
    create_and_execute_task ();

    //Add to link user variables
    TRACE_link_variable_add ("3", "Link_Utilization", 5.6);
    TRACE_link_variable_add ("6", "Link_Utilization", 7.8);
  }

  for (int i = 0; i < 10; i++) {
    create_and_execute_task ();

    //Subtract from link user variables
    TRACE_link_variable_sub ("3", "Link_Utilization", 3.4);
    TRACE_link_variable_sub ("6", "Link_Utilization", 5.6);
  }

  return 0;
}

int main(int argc, char *argv[])
{
  MSG_init(&argc, argv);
  xbt_assert(argc > 2, "Usage: %s platform_file deployment_file\n"
             "\tExample: %s msg_platform.xml msg_deployment.xml\n", argv[0], argv[0]);

  MSG_create_environment(argv[1]);

  //declaring link user variables (one without, another with a RGB color)
  TRACE_link_variable_declare("Link_Capacity");
  TRACE_link_variable_declare_with_color ("Link_Utilization", "0.9 0.1 0.1");

  //register functions and launch deployment
  MSG_function_register("master", trace_fun);
  MSG_function_register("worker", trace_fun);
  MSG_launch_application(argv[2]);

  MSG_main();
  return 0;
}
