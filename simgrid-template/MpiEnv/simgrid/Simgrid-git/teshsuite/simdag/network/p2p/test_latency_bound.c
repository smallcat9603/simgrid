/* Latency tests                                                            */

/* Copyright (c) 2007, 2009-2011, 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <stdio.h>
#include <stdlib.h>

#include "simdag/simdag.h"

#define TASK_NUM 3

/**
 * 3 tasks send 1 byte in parallel 
 * 3 flows exceed bandwidth
 * should be 10001.5
 * because the max tcp win size is 20000
 * 
 * @todo@ test assumes that max tcp win size is 20000
 * assert this
 */

int main(int argc, char **argv)
{
  int i;
  double time;
  double communication_amount[] = { 0.0, 1.0, 0.0, 0.0 };
  double no_cost[] = { 0.0, 0.0 };
  xbt_dynar_t ret;

  SD_task_t root;
  SD_task_t task[TASK_NUM];

  SD_init(&argc, argv);
  SD_create_environment(argv[1]);

  // xbt_assert( check max tcp win size, "MAX TCP WIN SIZE is 20000");

  root = SD_task_create("Root", NULL, 1.0);
  SD_task_schedule(root, 1, SD_workstation_get_list(), no_cost, no_cost,
                   -1.0);

  for (i = 0; i < TASK_NUM; i++) {
    task[i] = SD_task_create("Comm", NULL, 1.0);
    SD_task_schedule(task[i], 2, SD_workstation_get_list(), no_cost,
                     communication_amount, -1.0);
    SD_task_dependency_add(NULL, NULL, root, task[i]);
  }

  ret = SD_simulate(-1.0);
  xbt_dynar_free(&ret);

  time = SD_get_clock();

  printf("%g\n", time);
  fflush(stdout);

  for (i = 0; i < TASK_NUM; i++) {
    SD_task_destroy(task[i]);
  }
  SD_task_destroy(root);

  SD_exit();

  return 0;
}
