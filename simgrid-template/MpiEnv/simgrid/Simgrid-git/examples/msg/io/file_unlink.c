/* Copyright (c) 2008-2010, 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/** @addtogroup MSG_examples
 * 
 * - <b>io/file_unlink.c</b> TBA
 */

#define FILENAME1 "/home/doc/simgrid/examples/platforms/g5k.xml"

#include <stdio.h>
#include <stdlib.h>
#include "msg/msg.h"
#include "surf/surf_private.h"

int host(int argc, char *argv[]);

XBT_LOG_NEW_DEFAULT_CATEGORY(io_file,
                             "Messages specific for this io example");

int host(int argc, char *argv[])
{
  msg_file_t file = NULL;
  sg_size_t write;

  // First open
  XBT_INFO("\tOpen file '%s'",FILENAME1);
  file = MSG_file_open(FILENAME1, NULL);

  // Unlink the file
  XBT_INFO("\tUnlink file '%s'",MSG_file_get_name(file));
  MSG_file_unlink(file);

  // Re Open the file wich is in fact created
  XBT_INFO("\tOpen file '%s'",FILENAME1);
  file = MSG_file_open(FILENAME1, NULL);

  // Write into the new file
  write = MSG_file_write(file,100000);  // Write for 100Ko
  XBT_INFO("\tHave written %llu on %s",write,MSG_file_get_name(file));

  // Write into the new file
  write = MSG_file_write(file,100000);  // Write for 100Ko
  XBT_INFO("\tHave written %llu on %s",write,MSG_file_get_name(file));

  // Close the file
  XBT_INFO("\tClose file '%s'",MSG_file_get_name(file));
  MSG_file_close(file);

  return 0;
}

int main(int argc, char **argv)
{
  int res;
  MSG_init(&argc, argv);
  MSG_create_environment(argv[1]);
  xbt_dynar_t hosts =  MSG_hosts_as_dynar();
  MSG_function_register("host", host);
  unsigned long nb_hosts = xbt_dynar_length(hosts);
  XBT_INFO("Number of host '%lu'",nb_hosts);
  char* name_host = xbt_strdup("0");
  MSG_process_create( name_host, host, NULL, xbt_dynar_get_as(hosts,0,msg_host_t) );
  free(name_host);

  xbt_dynar_free(&hosts);

  res = MSG_main();
  XBT_INFO("Simulation time %g", MSG_get_clock());
  if (res == MSG_OK)
    return 0;
  else
    return 1;

}
