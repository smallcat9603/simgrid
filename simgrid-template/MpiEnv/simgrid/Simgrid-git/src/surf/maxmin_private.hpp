/* Copyright (c) 2004-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef _SURF_MAXMIN_PRIVATE_H
#define _SURF_MAXMIN_PRIVATE_H

#include "surf/maxmin.h"
#include "xbt/swag.h"
#include "xbt/mallocator.h"
#include "surf_interface.hpp"

/** @ingroup SURF_lmm
 * @brief LMM element
 */
typedef struct lmm_element {
  /* hookup to constraint */
  s_xbt_swag_hookup_t element_set_hookup;
  s_xbt_swag_hookup_t active_element_set_hookup;

  lmm_constraint_t constraint;
  lmm_variable_t variable;
  double value;
} s_lmm_element_t;
#define make_elem_active(elem) xbt_swag_insert_at_head(elem,&(elem->constraint->active_element_set))
#define make_elem_inactive(elem) xbt_swag_remove(elem,&(elem->constraint->active_element_set))

typedef struct lmm_constraint_light {
  double remaining_over_usage;
  lmm_constraint_t cnst;
} s_lmm_constraint_light_t;

/** @ingroup SURF_lmm
 * @brief LMM constraint
 */
typedef struct lmm_constraint {
  /* hookup to system */
  s_xbt_swag_hookup_t constraint_set_hookup;
  s_xbt_swag_hookup_t active_constraint_set_hookup;
  s_xbt_swag_hookup_t modified_constraint_set_hookup;
  s_xbt_swag_hookup_t saturated_constraint_set_hookup;

  s_xbt_swag_t element_set;     /* a list of lmm_element_t */
  s_xbt_swag_t active_element_set;      /* a list of lmm_element_t */
  double remaining;
  double usage;
  double bound;
  int shared;
  void *id;
  int id_int;
  double lambda;
  double new_lambda;
  lmm_constraint_light_t cnst_light;
} s_lmm_constraint_t;

/** @ingroup SURF_lmm
 * @brief LMM variable
 */
typedef struct lmm_variable {
  /* hookup to system */
  s_xbt_swag_hookup_t variable_set_hookup;
  s_xbt_swag_hookup_t saturated_variable_set_hookup;

  s_lmm_element_t *cnsts;
  int cnsts_size;
  int cnsts_number;
  double weight;
  double bound;
  double value;
  void *id;
  int id_int;
  unsigned visited;             /* used by lmm_update_modified_set */
  /* \begin{For Lagrange only} */
  double mu;
  double new_mu;
  double (*func_f) (struct lmm_variable * var, double x);       /* (f)    */
  double (*func_fp) (struct lmm_variable * var, double x);      /* (f')    */
  double (*func_fpi) (struct lmm_variable * var, double x);     /* (f')^{-1}    */
  /* \end{For Lagrange only} */
} s_lmm_variable_t;

/** @ingroup SURF_lmm
 * @brief LMM system
 */
typedef struct lmm_system {
  int modified;
  int selective_update_active;  /* flag to update partially the system only selecting changed portions */
  unsigned visited_counter;     /* used by lmm_update_modified_set */
  s_xbt_swag_t variable_set;    /* a list of lmm_variable_t */
  s_xbt_swag_t constraint_set;  /* a list of lmm_constraint_t */

  s_xbt_swag_t active_constraint_set;   /* a list of lmm_constraint_t */
  s_xbt_swag_t modified_constraint_set; /* a list of modified lmm_constraint_t */

  s_xbt_swag_t saturated_variable_set;  /* a list of lmm_variable_t */
  s_xbt_swag_t saturated_constraint_set;        /* a list of lmm_constraint_t_t */

  ActionLmmListPtr keep_track;

  xbt_mallocator_t variable_mallocator;
} s_lmm_system_t;

#define extract_variable(sys) xbt_swag_extract(&(sys->variable_set))
#define extract_constraint(sys) xbt_swag_extract(&(sys->constraint_set))
#define insert_constraint(sys,cnst) xbt_swag_insert(cnst,&(sys->constraint_set))
#define remove_variable(sys,var) do {xbt_swag_remove(var,&(sys->variable_set));\
                                 xbt_swag_remove(var,&(sys->saturated_variable_set));} while(0)
#define remove_constraint(sys,cnst) do {xbt_swag_remove(cnst,&(sys->constraint_set));\
                                        xbt_swag_remove(cnst,&(sys->saturated_constraint_set));} while(0)
#define make_constraint_active(sys,cnst) xbt_swag_insert(cnst,&(sys->active_constraint_set))
#define make_constraint_inactive(sys,cnst) \
  do { xbt_swag_remove(cnst, &sys->active_constraint_set);              \
    xbt_swag_remove(cnst, &sys->modified_constraint_set); } while (0)

/** @ingroup SURF_lmm
 * @brief Print informations about a lmm system
 * 
 * @param sys A lmm system
 */
void lmm_print(lmm_system_t sys);

extern double (*func_f_def) (lmm_variable_t, double);
extern double (*func_fp_def) (lmm_variable_t, double);
extern double (*func_fpi_def) (lmm_variable_t, double);

#endif                          /* _SURF_MAXMIN_PRIVATE_H */
