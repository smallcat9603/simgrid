/* Copyright (c) 2007-2012, 2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <stdio.h>
#include <stdlib.h>
#include "simdag/simdag.h"
#include "xbt/log.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(basic4, sd, "SimDag test basic4");

/* Basic SimDag Test 4
 * Scenario:
 *   - Create a chain of tasks (Init, A, Fin)
 *   - Have a 1B communication between two no-op tasks.
 * Verify that the tasks are actually simulated in the right order.
 * The simulated time should be equal to the network latency: 0.0001 seconds.
 */
int main(int argc, char **argv)
{
  /* creation of the tasks and their dependencies */

  SD_task_t taskInit;
  SD_task_t taskA;
  SD_task_t taskFin;
  xbt_dynar_t ret;

  /* scheduling parameters */

  double no_cost[] = { 0., 0., 0., 0. };
  double amount[] = { 0., 1., 0., 0. };

  /* initialisation of SD */
  SD_init(&argc, argv);

  /* creation of the environment */
  SD_create_environment(argv[1]);

  /* creation of the tasks and their dependencies */
  taskInit = SD_task_create("Task Init", NULL, 1.0);
  taskA = SD_task_create("Task A", NULL, 1.0);
  taskFin = SD_task_create("Task Fin", NULL, 1.0);


  /* let's launch the simulation! */
  SD_task_schedule(taskInit, 1, SD_workstation_get_list(), no_cost,
                   no_cost, -1.0);
  SD_task_schedule(taskA, 2, SD_workstation_get_list(), no_cost, amount,
                   -1.0);
  SD_task_schedule(taskFin, 1, SD_workstation_get_list(), no_cost, no_cost,
                   -1.0);

  SD_task_dependency_add(NULL, NULL, taskInit, taskA);
  SD_task_dependency_add(NULL, NULL, taskA, taskFin);

  ret = SD_simulate(-1.0);
  xbt_dynar_free(&ret);
  SD_task_destroy(taskInit);
  SD_task_destroy(taskA);
  SD_task_destroy(taskFin);

  XBT_INFO("Simulation time: %f", SD_get_clock());

  SD_exit();
  return 0;
}
