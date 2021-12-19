/* Copyright (c) 2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/** @addtogroup MSG_examples
 * 
 * - <b>io/remote.c</b> Example of delegated I/O operations
 */

#include <stdio.h>
#include <stdlib.h>
#include "msg/msg.h"
#include "surf/surf_private.h"

#define INMEGA (1024*1024)

int host(int argc, char *argv[]);

XBT_LOG_NEW_DEFAULT_CATEGORY(remote_io,
                             "Messages specific for this io example");


int host(int argc, char *argv[]){
  msg_file_t file = NULL;
  const char* filename;
  sg_size_t read, write;

  file = MSG_file_open(argv[1], NULL);
  filename = MSG_file_get_name(file);
  XBT_INFO("Opened file '%s'",filename);
  MSG_file_dump(file);

  XBT_INFO("Try to read %llu from '%s'",MSG_file_get_size(file),filename);
  read = MSG_file_read(file, MSG_file_get_size(file));
  XBT_INFO("Have read %llu from '%s'. Offset is now at: %llu",read,filename,
      MSG_file_tell(file));
  XBT_INFO("Seek back to the begining of the stream...");
  MSG_file_seek(file, 0, SEEK_SET);
  XBT_INFO("Offset is now at: %llu", MSG_file_tell(file));

  MSG_file_close(file);

  if (argc > 5){
    file = MSG_file_open(argv[2], NULL);
    filename = MSG_file_get_name(file);
    XBT_INFO("Opened file '%s'",filename);
    XBT_INFO("Try to write %llu MiB to '%s'",
        MSG_file_get_size(file)/1024,
        filename);
    write = MSG_file_write(file, MSG_file_get_size(file)*1024);
    XBT_INFO("Have written %llu bytes to '%s'.",write,filename);

    msg_host_t src, dest;
    src= MSG_host_self();
    dest = MSG_get_host_by_name(argv[3]);
    if (atoi(argv[5])){
      XBT_INFO("Move '%s' (of size %llu) from '%s' to '%s'", filename,
           MSG_file_get_size(file), MSG_host_get_name(src),
           argv[3]);
      MSG_file_rmove(file, dest, argv[4]);
    } else {
      XBT_INFO("Copy '%s' (of size %llu) from '%s' to '%s'", filename,
           MSG_file_get_size(file), MSG_host_get_name(src),
           argv[3]);
      MSG_file_rcopy(file, dest, argv[4]);
      MSG_file_close(file);
    }
  }

  return 0;
}



int main(int argc, char **argv)
{
  int res;
  unsigned int cur;
  xbt_dynar_t storages;
  msg_storage_t st;

  MSG_init(&argc, argv);
  MSG_create_environment(argv[1]);
  MSG_function_register("host", host);
  MSG_launch_application(argv[2]);

  storages = MSG_storages_as_dynar();
  xbt_dynar_foreach(storages, cur, st){
    XBT_INFO("Init: %llu MiB used on '%s'",
        MSG_storage_get_used_size(st)/INMEGA,
        MSG_storage_get_name(st));
  }

  res = MSG_main();

  xbt_dynar_foreach(storages, cur, st){
    XBT_INFO("Init: %llu MiB used on '%s'",
        MSG_storage_get_used_size(st)/INMEGA,
        MSG_storage_get_name(st));
  }
  xbt_dynar_free_container(&storages);

  XBT_INFO("Simulation time %g", MSG_get_clock());
  if (res == MSG_OK)
    return 0;
  else
    return 1;
}
