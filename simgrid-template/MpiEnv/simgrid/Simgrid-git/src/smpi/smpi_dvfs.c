/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt/log.h"
#include "simgrid/simix.h"
#include "smpi/smpi.h"
#include "internal_config.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(smpi_dvfs, smpi,
                                "Logging specific to SMPI (experimental DVFS support)");

/**
 * \brief Return the speed of the processor (in flop/s) at a given pstate
 *
 * \param pstate_index pstate to test
 * \return Returns the processor speed associated with pstate_index
 */
double smpi_get_host_power_peak_at(int pstate_index)
{
  return simcall_host_get_power_peak_at(SIMIX_host_self(), pstate_index);
}

/**
 * \brief Return the current speed of the processor (in flop/s)
 *
 * \return Returns the current processor speed
 */
double smpi_get_host_current_power_peak(void)
{
  return simcall_host_get_current_power_peak(SIMIX_host_self());
}

/**
 * \brief Return the number of pstates defined for host
 */
int smpi_get_host_nb_pstates(void)
{
  return simcall_host_get_nb_pstates(SIMIX_host_self());
}

/**
 * \brief Sets the speed of the processor (in flop/s) at a given pstate
 *
 * \param pstate_index pstate to switch to
 */
void smpi_set_host_power_peak_at(int pstate_index)
{
  simcall_host_set_power_peak_at(SIMIX_host_self(), pstate_index);
}

/**
 * \brief Return the total energy consumed by a host (in Joules)
 *
 * \return Returns the consumed energy
 */
double smpi_get_host_consumed_energy(void)
{
  return simcall_host_get_consumed_energy(SIMIX_host_self());
}


#ifdef SMPI_FORTRAN

#if defined(__alpha__) || defined(__sparc64__) || defined(__x86_64__) || defined(__ia64__)
typedef int integer;
typedef unsigned int uinteger;
#else
typedef long int integer;
typedef unsigned long int uinteger;
#endif
typedef char *address;
typedef short int shortint;
typedef float real;
typedef double doublereal;
typedef struct { real r, i; } complex;
typedef struct { doublereal r, i; } doublecomplex;

XBT_PUBLIC(doublereal) smpi_get_host_power_peak_at_(integer *pstate_index);
doublereal smpi_get_host_power_peak_at_(integer *pstate_index)
{
  return (doublereal)smpi_get_host_power_peak_at((int)*pstate_index);
}

XBT_PUBLIC(doublereal) smpi_get_host_current_power_peak_(void);
doublereal smpi_get_host_current_power_peak_(void)
{
  return smpi_get_host_current_power_peak();
}

XBT_PUBLIC(integer) smpi_get_host_nb_pstates_(void);
integer smpi_get_host_nb_pstates_(void)
{
  return (integer)smpi_get_host_nb_pstates();
}

XBT_PUBLIC(void) smpi_set_host_power_peak_at_(integer *pstate_index);
void smpi_set_host_power_peak_at_(integer *pstate_index)
{
  smpi_set_host_power_peak_at((int)*pstate_index);
}

XBT_PUBLIC(doublereal) smpi_get_host_consumed_energy_(void);
doublereal smpi_get_host_consumed_energy_(void)
{
  return (doublereal)smpi_get_host_consumed_energy();
}

#endif
