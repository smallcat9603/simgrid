/* Copyright (c) 2007-2010, 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef _POPPING_PRIVATE_H
#define _POPPING_PRIVATE_H

SG_BEGIN_DECL()

/********************************* Simcalls *********************************/
XBT_PUBLIC(const char*) simcall_names[]; /* Name of each simcall */

#include "popping_enum.h" /* Definition of e_smx_simcall_t, with one value per simcall */
#include "mc/mc_interface.h" /* Definition of mc_snapshot_t, used by one simcall */

typedef int (*simix_match_func_t)(void *, void *, smx_synchro_t);
typedef void (*simix_copy_data_func_t)(smx_synchro_t, void*, size_t);
typedef void (*simix_clean_func_t)(void *);
typedef void (*FPtr)(void); // Hide the ugliness

/* Pack all possible scalar types in an union */
union u_smx_scalar {
  char            c;
  const char*     cc;
  short           s;
  int             i;
  long            l;
  unsigned char   uc;
  unsigned short  us;
  unsigned int    ui;
  unsigned long   ul;
  float           f;
  double          d;
  size_t          sz;
  sg_size_t       sgsz;
  sg_offset_t     sgoff;
  void*           dp;
  FPtr            fp;
  const void*     cp;
};

/**
 * \brief Represents a simcall to the kernel.
 */
typedef struct s_smx_simcall {
  e_smx_simcall_t call;
  smx_process_t issuer;
#ifdef HAVE_MC
  int mc_value;
#endif
  union u_smx_scalar args[11];
  union u_smx_scalar result;
} s_smx_simcall_t, *smx_simcall_t;

#if HAVE_MC
#define SIMCALL_SET_MC_VALUE(simcall, value) ((simcall)->mc_value = (value))
#define SIMCALL_GET_MC_VALUE(simcall) ((simcall)->mc_value)
#else
#define SIMCALL_SET_MC_VALUE(simcall, value) ((void)0)
#define SIMCALL_GET_MC_VALUE(simcall) 0
#endif

#include "popping_accessors.h"

/******************************** General *************************************/

void SIMIX_simcall_answer(smx_simcall_t);
void SIMIX_simcall_handle(smx_simcall_t, int);
void SIMIX_simcall_exit(smx_synchro_t);
const char *SIMIX_simcall_name(e_smx_simcall_t kind);

SG_END_DECL()

#endif
