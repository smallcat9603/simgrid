/* Copyright (c) 2007-2020. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/simdag.h"
#include "xbt/asserts.h"
#include "xbt/log.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(basic6, sd, "SimDag test basic6");
/* test scheduling 2 tasks at the same time without artificial dependencies */

/* Basic SimDag Test 6
 * Scenario:
 *   - Schedule two parallel tasks concurrently on a P2P platform
 *   - Hosts computes 1B per second
 * Computing power is shared between tasks.
 * Simulated time should be:
 *          1/(1/2) = 2 seconds
 */

int main(int argc, char **argv)
{
  double comm_cost[] = { 0.0, 0.0, 0.0, 0.0 };
  double comp_cost[] = { 1.0 };
  xbt_dynar_t ret    = xbt_dynar_new(sizeof(SD_task_t), NULL);

  SD_init(&argc, argv);
  SD_create_environment(argv[1]);

  SD_task_t taskA = SD_task_create("Task A", NULL, 1.0);
  SD_task_t taskB = SD_task_create("Task B", NULL, 1.0);

  sg_host_t *hosts = sg_host_list();
  SD_task_schedule(taskA, 1, hosts, comp_cost, comm_cost, -1.0);
  SD_task_schedule(taskB, 1, hosts, comp_cost, comm_cost, -1.0);
  xbt_free(hosts);

  SD_simulate_with_update(-1.0, ret);
  xbt_assert(xbt_dynar_length(ret) == 2, "I was expecting the completion of 2 tasks, but I got %lu instead",
             xbt_dynar_length(ret));
  SD_task_destroy(taskA);
  SD_task_destroy(taskB);
  xbt_dynar_free(&ret);

  XBT_INFO("Simulation time: %f", SD_get_clock());

  return 0;
}
