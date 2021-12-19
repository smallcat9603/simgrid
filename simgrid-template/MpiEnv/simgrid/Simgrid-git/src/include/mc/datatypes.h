/* Copyright (c) 2008-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef MC_DATATYPE_H
#define MC_DATATYPE_H

#define UNW_LOCAL_ONLY

#include "xbt/misc.h"
#include "xbt/swag.h"
#include "xbt/fifo.h"

#include <libunwind.h>
#include <dwarf.h>

SG_BEGIN_DECL()

/******************************* Transitions **********************************/

typedef struct s_mc_transition *mc_transition_t;


/*********** Structures for snapshot comparison **************************/

typedef struct s_mc_heap_ignore_region{
  int block;
  int fragment;
  void *address;
  size_t size;
}s_mc_heap_ignore_region_t, *mc_heap_ignore_region_t;

typedef struct s_stack_region{
  void *address;
  char *process_name;
  void *context;
  size_t size;
  int block;
  int process_index;
}s_stack_region_t, *stack_region_t;

void heap_ignore_region_free(mc_heap_ignore_region_t r);
void heap_ignore_region_free_voidp(void *r);

/************ Object info *************/

typedef struct s_mc_object_info s_mc_object_info_t, *mc_object_info_t;

/************ DWARF structures *************/

typedef int e_dw_type_type;

typedef struct s_dw_type s_dw_type_t, *dw_type_t;

char* get_type_description(mc_object_info_t info, char *type_name);

SG_END_DECL()
#endif                          /* _MC_MC_H */
