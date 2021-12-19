/* Copyright (c) 2007-2012, 2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <stdio.h>
#include <stdlib.h>
#include "simdag/simdag.h"
#include "xbt/log.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(basic2, sd, "SimDag test basic2");

/* Basic SimDag Test 2
 * Scenario:
 *   - Create a no-op Init task
 *   - Create two communication tasks: 1GB and 100MB
 *   - Schedule them concurrently on the two hosts of the platform
 * The two communications occur simultaneously. They share the network for the
 * duration of the shortest one, then the longest one has the full bandwidth.
 * Simulated time should be:
 *          1e8/(1/2*1.25e8) + 9e8/1.25e8) + 1e-4 = 8.8001 seconds
 */
int main(int argc, char **argv)
{

  SD_task_t taskInit;
  SD_task_t taskA;
  SD_task_t taskB;
  xbt_dynar_t ret;

  const SD_workstation_t *workstation;

  double communication_amount1 = 1e9;
  double communication_amount2 = 1e8;
  double no_cost = 0.0;

  /* initialization of SD */
  SD_init(&argc, argv);

  /* creation of the environment */
  SD_create_environment(argv[1]);

  /* creation of the tasks and their dependencies */
  taskInit = SD_task_create("Init", NULL, 1.0);
  taskA = SD_task_create("Task Comm A", NULL, 1.0);
  taskB = SD_task_create("Task Comm B", NULL, 1.0);


  /* scheduling parameters */

  workstation = SD_workstation_get_list();

  /* let's launch the simulation! */
  SD_task_schedule(taskInit, 1, SD_workstation_get_list(), &no_cost,
                   &no_cost, -1.0);
  SD_task_schedule(taskA, 1, &workstation[0], &no_cost,
                   &communication_amount1, -1.0);
  SD_task_schedule(taskB, 1, &workstation[1], &no_cost,
                   &communication_amount2, -1.0);

  SD_task_dependency_add(NULL, NULL, taskInit, taskA);
  SD_task_dependency_add(NULL, NULL, taskInit, taskB);

  ret = SD_simulate(-1.0);
  xbt_dynar_free(&ret);
  SD_task_destroy(taskA);
  SD_task_destroy(taskB);
  SD_task_destroy(taskInit);

  XBT_INFO("Simulation time: %f", SD_get_clock());

  SD_exit();
  return 0;
}
