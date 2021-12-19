/* Copyright (c) 2010-2012, 2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef JEDULE_OUTPUT_H_
#define JEDULE_OUTPUT_H_

#include "simgrid_config.h"

#include <stdio.h>

#include "jedule_events.h"
#include "jedule_platform.h"

#ifdef HAVE_JEDULE

extern xbt_dynar_t jedule_event_list;

void jedule_init_output(void);

void jedule_cleanup_output(void);

void jedule_store_event(jed_event_t event);

void write_jedule_output(FILE *file, jedule_t jedule,
    xbt_dynar_t event_list, xbt_dict_t meta_info_dict);

#endif

#endif /* JEDULE_OUTPUT_H_ */
