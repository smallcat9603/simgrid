/* Copyright (c) 2010-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt/asserts.h"
#include "xbt/dynar.h"

#include "surf/surf_private.h"
#include "surf/surf_resource.h"
#include "surf/surf.h"

#include "instr/jedule/jedule_sd_binding.h"
#include "instr/jedule/jedule_events.h"
#include "instr/jedule/jedule_platform.h"
#include "instr/jedule/jedule_output.h"

#include "simdag/private.h"

#include <stdio.h>

#ifdef HAVE_JEDULE

XBT_LOG_NEW_CATEGORY(jedule, "Logging specific to Jedule");
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(jed_sd, jedule,
                                "Logging specific to Jedule SD binding");

jedule_t jedule;

void jedule_log_sd_event(SD_task_t task)
{
  xbt_dynar_t host_list;
  jed_event_t event;
  int i;

  xbt_assert(task != NULL);

  host_list = xbt_dynar_new(sizeof(char*), NULL);

  for(i=0; i<task->workstation_nb; i++) {
    char *hostname = sg_host_name(task->workstation_list[i]);
    xbt_dynar_push(host_list, &hostname);
  }

  create_jed_event(&event,
      (char*)SD_task_get_name(task),
      task->start_time,
      task->finish_time,
      "SD");

  jed_event_add_resources(event, host_list);
  jedule_store_event(event);

  xbt_dynar_free(&host_list);
}

static void create_hierarchy(AS_t current_comp,
                             jed_simgrid_container_t current_container)
{
  xbt_dict_cursor_t cursor = NULL;
  char *key;
  AS_t elem;
  xbt_dict_t routing_sons = surf_AS_get_routing_sons(current_comp);

  if (xbt_dict_is_empty(routing_sons)) {
    // I am no AS
    // add hosts to jedule platform
    xbt_dynar_t table = surf_AS_get_hosts(current_comp);
    xbt_dynar_t hosts;
    unsigned int dynar_cursor;
    sg_host_t host_elem;

    hosts = xbt_dynar_new(sizeof(char*), NULL);

    xbt_dynar_foreach(table, dynar_cursor, host_elem) {
      xbt_dynar_push_as(hosts, char*, sg_host_name(host_elem));
    }

    jed_simgrid_add_resources(current_container, hosts);
    xbt_dynar_free(&hosts);
    xbt_dynar_free(&table);
  } else {
    xbt_dict_foreach(routing_sons, cursor, key, elem) {
      jed_simgrid_container_t child_container;
      jed_simgrid_create_container(&child_container, surf_AS_get_name(elem));
      jed_simgrid_add_container(current_container, child_container);
      XBT_DEBUG("name : %s\n", surf_AS_get_name(elem));
      create_hierarchy(elem, child_container);
    }
  }
}

void jedule_setup_platform()
{
  AS_t root_comp;
  // e_surf_network_element_type_t type;

  jed_simgrid_container_t root_container;

  jed_create_jedule(&jedule);

  root_comp = surf_AS_get_routing_root();
  XBT_DEBUG("root name %s\n", surf_AS_get_name(root_comp));

  jed_simgrid_create_container(&root_container, surf_AS_get_name(root_comp));
  jedule->root_container = root_container;

  create_hierarchy(root_comp, root_container);
}


void jedule_sd_cleanup()
{
  jedule_cleanup_output();
}

void jedule_sd_init()
{
  jedule_init_output();
}

void jedule_sd_exit(void)
{
  if (jedule) {
    jed_free_jedule(jedule);
    jedule = NULL;
  }
}

void jedule_sd_dump()
{
  if (jedule) {
    FILE *fh;
    char fname[1024];

    fname[0] = '\0';
    strcat(fname, xbt_binary_name);
    strcat(fname, ".jed\0");

    fh = fopen(fname, "w");

    write_jedule_output(fh, jedule, jedule_event_list, NULL);

    fclose(fh);
  }
}

#endif
