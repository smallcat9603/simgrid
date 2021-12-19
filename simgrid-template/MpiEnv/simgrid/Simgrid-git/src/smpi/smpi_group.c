/* Copyright (c) 2010, 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "private.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(smpi_group, smpi,
                                "Logging specific to SMPI (group)");

typedef struct s_smpi_mpi_group {
  int size;
  int *rank_to_index_map;
  int *index_to_rank_map;
  int refcount;
} s_smpi_mpi_group_t;

static s_smpi_mpi_group_t mpi_MPI_GROUP_EMPTY = {
  0,                            /* size */
  NULL,                         /* rank_to_index_map */
  NULL,                         /* index_to_rank_map */
  1,                            /* refcount: start > 0 so that this group never gets freed */
};

MPI_Group MPI_GROUP_EMPTY = &mpi_MPI_GROUP_EMPTY;

MPI_Group smpi_group_new(int size)
{
  MPI_Group group;
  int i, count;

  count = SIMIX_process_count();
  group = xbt_new(s_smpi_mpi_group_t, 1);
  group->size = size;
  group->rank_to_index_map = xbt_new(int, size);
  group->index_to_rank_map = xbt_new(int, count);
  group->refcount = 1;
  for (i = 0; i < size; i++) {
    group->rank_to_index_map[i] = MPI_UNDEFINED;
  }
  for (i = 0; i < count; i++) {
    group->index_to_rank_map[i] = MPI_UNDEFINED;
  }

  return group;
}

MPI_Group smpi_group_copy(MPI_Group origin)
{
  MPI_Group group=origin;
  int i, count;
  if(origin!= smpi_comm_group(MPI_COMM_WORLD)
            && origin != MPI_GROUP_NULL
            && origin != smpi_comm_group(MPI_COMM_SELF)
            && origin != MPI_GROUP_EMPTY)
    {
      count = smpi_process_count();
      group = xbt_new(s_smpi_mpi_group_t, 1);
      group->size = origin->size;
      group->rank_to_index_map = xbt_new(int, group->size);
      group->index_to_rank_map = xbt_new(int, count);
      group->refcount = 1;
      for (i = 0; i < group->size; i++) {
        group->rank_to_index_map[i] = origin->rank_to_index_map[i];
      }
      for (i = 0; i < count; i++) {
        group->index_to_rank_map[i] = origin->index_to_rank_map[i];
      }
    }

  return group;
}


void smpi_group_destroy(MPI_Group group)
{
  if(group!= smpi_comm_group(MPI_COMM_WORLD)
          && group != MPI_GROUP_NULL
          && group != smpi_comm_group(MPI_COMM_SELF)
          && group != MPI_GROUP_EMPTY)
  smpi_group_unuse(group);
}

void smpi_group_set_mapping(MPI_Group group, int index, int rank)
{
  if (rank < group->size && index < SIMIX_process_count()) {
    group->rank_to_index_map[rank] = index;
    if(index!=MPI_UNDEFINED)group->index_to_rank_map[index] = rank;
  }
}

int smpi_group_index(MPI_Group group, int rank)
{
  int index = MPI_UNDEFINED;

  if (0 <= rank && rank < group->size) {
    index = group->rank_to_index_map[rank];
  }
  return index;
}

int smpi_group_rank(MPI_Group group, int index)
{
  int rank = MPI_UNDEFINED;
  rank = group->index_to_rank_map[index];
  return rank;
}

int smpi_group_use(MPI_Group group)
{
  group->refcount++;
  return group->refcount;
}

int smpi_group_unuse(MPI_Group group)
{
  group->refcount--;
  if (group->refcount <= 0) {
    xbt_free(group->rank_to_index_map);
    xbt_free(group->index_to_rank_map);
    xbt_free(group);
    return 0;
  }
  return group->refcount;

}

int smpi_group_size(MPI_Group group)
{
  return group->size;
}

int smpi_group_compare(MPI_Group group1, MPI_Group group2)
{
  int result;
  int i, index, rank, size;

  result = MPI_IDENT;
  if (smpi_group_size(group1) != smpi_group_size(group2)) {
    result = MPI_UNEQUAL;
  } else {
    size = smpi_group_size(group2);
    for (i = 0; i < size; i++) {
      index = smpi_group_index(group1, i);
      rank = smpi_group_rank(group2, index);
      if (rank == MPI_UNDEFINED) {
        result = MPI_UNEQUAL;
        break;
      }
      if (rank != i) {
        result = MPI_SIMILAR;
      }
    }
  }
  return result;
}

int smpi_group_incl(MPI_Group group, int n, int* ranks, MPI_Group* newgroup)
{
  int i=0, index=0;
  if (n == 0) {
    *newgroup = MPI_GROUP_EMPTY;
  } else if (n == smpi_group_size(group)) {
    *newgroup = group;
    if(group!= smpi_comm_group(MPI_COMM_WORLD)
              && group != MPI_GROUP_NULL
              && group != smpi_comm_group(MPI_COMM_SELF)
              && group != MPI_GROUP_EMPTY)
    smpi_group_use(group);
  } else {
    *newgroup = smpi_group_new(n);
    for (i = 0; i < n; i++) {
      index = smpi_group_index(group, ranks[i]);
      smpi_group_set_mapping(*newgroup, index, i);
    }
  }
  return MPI_SUCCESS;
}
