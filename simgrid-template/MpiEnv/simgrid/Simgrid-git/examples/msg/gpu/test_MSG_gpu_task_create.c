/* Copyright (c) 2007-2010, 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/** @addtogroup MSG_examples
 * 
 * - <b>gpu/test_MSG_gpu_task_create.c</b> Example of use of the very experimental (for now) GPU resource. 
 */

#include <stdio.h>
#include <stdlib.h>
#include "msg/msg.h"
#include "xbt/log.h"
#include "xbt/asserts.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(msg_test,
                             "Messages specific for GPU msg example");


/** Main function */
int main(int argc, char *argv[])
{
  msg_error_t res = MSG_OK;

  MSG_init(&argc, argv);

  msg_gpu_task_t mytask = NULL;

  mytask = MSG_gpu_task_create("testTask", 2000.0, 20.0, 20.0);

  XBT_INFO("GPU task %p was created", mytask);

  if (res == MSG_OK)
    return 0;
  else
    return 1;
}                               /* end_of_main */
