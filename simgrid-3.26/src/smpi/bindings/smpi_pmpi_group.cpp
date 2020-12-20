/* Copyright (c) 2007-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "private.hpp"
#include "smpi_coll.hpp"
#include "smpi_comm.hpp"
#include "smpi_datatype_derived.hpp"
#include "smpi_op.hpp"
#include "src/smpi/include/smpi_actor.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(smpi_pmpi);

/* PMPI User level calls */

int PMPI_Group_free(MPI_Group * group)
{
  CHECK_NULL(1, MPI_ERR_ARG, group)
  if(*group != MPI_COMM_WORLD->group() && *group != MPI_GROUP_EMPTY)
    simgrid::smpi::Group::unref(*group);
  *group = MPI_GROUP_NULL;
  return MPI_SUCCESS;
}

int PMPI_Group_size(MPI_Group group, int *size)
{
  CHECK_GROUP(1, group)
  CHECK_NULL(2, MPI_ERR_ARG, size)
  *size = group->size();
  return MPI_SUCCESS;
}

int PMPI_Group_rank(MPI_Group group, int *rank)
{
  CHECK_GROUP(1, group)
  CHECK_NULL(2, MPI_ERR_ARG, rank)
  *rank = group->rank(simgrid::s4u::this_actor::get_pid());
  return MPI_SUCCESS;
}

int PMPI_Group_translate_ranks(MPI_Group group1, int n, const int *ranks1, MPI_Group group2, int *ranks2)
{
  CHECK_GROUP(1, group1)
  CHECK_GROUP(4, group2)
  for (int i = 0; i < n; i++) {
    if(ranks1[i]==MPI_PROC_NULL){
      ranks2[i]=MPI_PROC_NULL;
    }else{
      simgrid::s4u::Actor* actor = group1->actor(ranks1[i]);
      ranks2[i] = group2->rank(actor);
    }
  }
  return MPI_SUCCESS;
}

int PMPI_Group_compare(MPI_Group group1, MPI_Group group2, int *result)
{
  CHECK_GROUP(1, group1)
  CHECK_GROUP(2, group2)
  CHECK_NULL(3, MPI_ERR_ARG, result)
  *result = group1->compare(group2);
  return MPI_SUCCESS;
}

int PMPI_Group_union(MPI_Group group1, MPI_Group group2, MPI_Group * newgroup)
{
  CHECK_GROUP(1, group1)
  CHECK_GROUP(2, group2)
  CHECK_NULL(3, MPI_ERR_ARG, newgroup)
  return group1->group_union(group2, newgroup);
}

int PMPI_Group_intersection(MPI_Group group1, MPI_Group group2, MPI_Group * newgroup)
{
  CHECK_GROUP(1, group1)
  CHECK_GROUP(2, group2)
  CHECK_NULL(3, MPI_ERR_ARG, newgroup)
  return group1->intersection(group2,newgroup);
}

int PMPI_Group_difference(MPI_Group group1, MPI_Group group2, MPI_Group * newgroup)
{
  CHECK_GROUP(1, group1)
  CHECK_GROUP(2, group2)
  CHECK_NULL(3, MPI_ERR_ARG, newgroup)
  return group1->difference(group2,newgroup);
}

int PMPI_Group_incl(MPI_Group group, int n, const int *ranks, MPI_Group * newgroup)
{
  CHECK_GROUP(1, group)
  CHECK_NULL(4, MPI_ERR_ARG, newgroup)
  return group->incl(n, ranks, newgroup);
}

int PMPI_Group_excl(MPI_Group group, int n, const int *ranks, MPI_Group * newgroup)
{
  CHECK_GROUP(1, group)
  CHECK_NULL(4, MPI_ERR_ARG, newgroup)
  if (n == 0) {
    *newgroup = group;
    if (group != MPI_COMM_WORLD->group() && group != MPI_COMM_SELF->group() && group != MPI_GROUP_EMPTY)
      group->ref();
    return MPI_SUCCESS;
  } else if (n == group->size()) {
    *newgroup = MPI_GROUP_EMPTY;
    return MPI_SUCCESS;
  } else {
    return group->excl(n,ranks,newgroup);
  }
}

int PMPI_Group_range_incl(MPI_Group group, int n, int ranges[][3], MPI_Group * newgroup)
{
  CHECK_GROUP(1, group)
  CHECK_NULL(4, MPI_ERR_ARG, newgroup)
  if (n == 0) {
    *newgroup = MPI_GROUP_EMPTY;
    return MPI_SUCCESS;
  } else {
    return group->range_incl(n,ranges,newgroup);
  }
}

int PMPI_Group_range_excl(MPI_Group group, int n, int ranges[][3], MPI_Group * newgroup)
{
  CHECK_GROUP(1, group)
  CHECK_NULL(4, MPI_ERR_ARG, newgroup)
  if (n == 0) {
    *newgroup = group;
    if (group != MPI_COMM_WORLD->group() && group != MPI_COMM_SELF->group() &&
        group != MPI_GROUP_EMPTY)
      group->ref();
    return MPI_SUCCESS;
  } else {
    return group->range_excl(n,ranges,newgroup);
  }
}

MPI_Group PMPI_Group_f2c(MPI_Fint group){
  if(group==-1)
    return MPI_GROUP_NULL;
  return simgrid::smpi::Group::f2c(group);
}

MPI_Fint PMPI_Group_c2f(MPI_Group group){
  if(group==MPI_GROUP_NULL)
    return -1;
  return group->c2f();
}
