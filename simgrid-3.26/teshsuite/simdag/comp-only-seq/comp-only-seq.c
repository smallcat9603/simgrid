/* Copyright (c) 2007-2020. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <stdio.h>
#include <stdlib.h>

#include "simgrid/simdag.h"

int main(int argc, char **argv)
{

  double comm_amount[] = { 0.0 };
  double comp_cost[] = { 1.0 };

  SD_init(&argc, argv);
  SD_create_environment(argv[1]);

  SD_task_t task = SD_task_create("seqtask", NULL, 1.0);
  sg_host_t *hosts = sg_host_list();
  SD_task_schedule(task, 1, hosts, comp_cost, comm_amount, -1.0);
  xbt_free(hosts);

  SD_simulate(-1.0);

  printf("%g\n", SD_get_clock());
  fflush(stdout);

  SD_task_destroy(task);

  return 0;
}
