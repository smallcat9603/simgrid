/* Copyright (c) 2008-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <sys/stat.h>
#include <fcntl.h>

#include "xbt/log.h"

#include "mc/mc.h"
#include "src/mc/mc_private.hpp"
#include "src/xbt/mmalloc/mmprivate.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(mc_memory, mc,
                                "Logging specific to MC (memory)");

/* Initialize the model-checker memory subsystem */
/* It creates the two heap regions: std_heap and mc_heap */
void MC_memory_init()
{
  if (not malloc_use_mmalloc())
    xbt_die("Model-checking support is not enabled: run with simgrid-mc.");
}
