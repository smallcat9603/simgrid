/* Copyright (c) 2007-2011, 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt/sysdep.h"
#include "xbt/log.h"
#include "maxmin_private.hpp"
#include <stdlib.h>
#include <math.h>

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(surf_maxmin);
#define SHOW_EXPR_G(expr) XBT_DEBUG(#expr " = %g",expr);
#define SHOW_EXPR_D(expr) XBT_DEBUG(#expr " = %d",expr);
#define SHOW_EXPR_P(expr) XBT_DEBUG(#expr " = %p",expr);

void bottleneck_solve(lmm_system_t sys)
{
  void *_var, *_var_next, *_cnst, *_cnst_next, *_elem;
  lmm_variable_t var = NULL;
  lmm_constraint_t cnst = NULL;
  s_lmm_constraint_t s_cnst;
  lmm_element_t elem = NULL;
  xbt_swag_t cnst_list = NULL;
  xbt_swag_t var_list = NULL;
  xbt_swag_t elem_list = NULL;
  int i;

  static s_xbt_swag_t cnst_to_update;

  if (!(sys->modified))
    return;

  /* Init */
  xbt_swag_init(&(cnst_to_update),
                xbt_swag_offset(s_cnst, saturated_constraint_set_hookup));

  var_list = &(sys->variable_set);
  XBT_DEBUG("Variable set : %d", xbt_swag_size(var_list));
  xbt_swag_foreach(_var, var_list) {
	var = (lmm_variable_t)_var;
    int nb = 0;
    var->value = 0.0;
    XBT_DEBUG("Handling variable %p", var);
    xbt_swag_insert(var, &(sys->saturated_variable_set));
    for (i = 0; i < var->cnsts_number; i++) {
      if (var->cnsts[i].value == 0.0)
        nb++;
    }
    if ((nb == var->cnsts_number) && (var->weight > 0.0)) {
      XBT_DEBUG("Err, finally, there is no need to take care of variable %p",
             var);
      xbt_swag_remove(var, &(sys->saturated_variable_set));
      var->value = 1.0;
    }
    if (var->weight <= 0.0) {
      XBT_DEBUG("Err, finally, there is no need to take care of variable %p",
             var);
      xbt_swag_remove(var, &(sys->saturated_variable_set));
    }
  }
  var_list = &(sys->saturated_variable_set);

  cnst_list = &(sys->active_constraint_set);
  XBT_DEBUG("Active constraints : %d", xbt_swag_size(cnst_list));
  xbt_swag_foreach(_cnst, cnst_list) {
	cnst = (lmm_constraint_t)_cnst;
    xbt_swag_insert(cnst, &(sys->saturated_constraint_set));
  }
  cnst_list = &(sys->saturated_constraint_set);
  xbt_swag_foreach(_cnst, cnst_list) {
	cnst = (lmm_constraint_t)_cnst;
    cnst->remaining = cnst->bound;
    cnst->usage = 0.0;
  }

  XBT_DEBUG("Fair bottleneck Initialized");

  /* 
   * Compute Usage and store the variables that reach the maximum.
   */
  do {
    if (XBT_LOG_ISENABLED(surf_maxmin, xbt_log_priority_debug)) {
      XBT_DEBUG("Fair bottleneck done");
      lmm_print(sys);
    }
    XBT_DEBUG("******* Constraints to process: %d *******",
           xbt_swag_size(cnst_list));
    xbt_swag_foreach_safe(_cnst, _cnst_next, cnst_list) {
      cnst = (lmm_constraint_t)_cnst;
      int nb = 0;
      XBT_DEBUG("Processing cnst %p ", cnst);
      elem_list = &(cnst->element_set);
      cnst->usage = 0.0;
      xbt_swag_foreach(_elem, elem_list) {
    	elem = (lmm_element_t)_elem;
        if (elem->variable->weight <= 0)
          break;
        if ((elem->value > 0)
            && xbt_swag_belongs(elem->variable, var_list))
          nb++;
      }
      XBT_DEBUG("\tThere are %d variables", nb);
      if (nb > 0 && !cnst->shared)
        nb = 1;
      if (!nb) {
        cnst->remaining = 0.0;
        cnst->usage = cnst->remaining;
        xbt_swag_remove(cnst, cnst_list);
        continue;
      }
      cnst->usage = cnst->remaining / nb;
      XBT_DEBUG("\tConstraint Usage %p : %f with %d variables", cnst,
             cnst->usage, nb);
    }

    xbt_swag_foreach_safe(_var, _var_next, var_list) {
      var = (lmm_variable_t)_var;
      double min_inc =
          var->cnsts[0].constraint->usage / var->cnsts[0].value;
      for (i = 1; i < var->cnsts_number; i++) {
        lmm_element_t elm = &var->cnsts[i];
        min_inc = MIN(min_inc, elm->constraint->usage / elm->value);
      }
      if (var->bound > 0)
        min_inc = MIN(min_inc, var->bound - var->value);
      var->mu = min_inc;
      XBT_DEBUG("Updating variable %p maximum increment: %g", var, var->mu);
      var->value += var->mu;
      if (var->value == var->bound) {
        xbt_swag_remove(var, var_list);
      }
    }

    xbt_swag_foreach_safe(_cnst, _cnst_next, cnst_list) {
      cnst = (lmm_constraint_t)_cnst;
      XBT_DEBUG("Updating cnst %p ", cnst);
      elem_list = &(cnst->element_set);
      xbt_swag_foreach(_elem, elem_list) {
        elem = (lmm_element_t)_elem;
        if (elem->variable->weight <= 0)
          break;
        if (cnst->shared) {
          XBT_DEBUG("\tUpdate constraint %p (%g) with variable %p by %g",
                 cnst, cnst->remaining, elem->variable,
                 elem->variable->mu);
          double_update(&(cnst->remaining),
                        elem->value * elem->variable->mu, sg_maxmin_precision);
        } else {
          XBT_DEBUG
              ("\tNon-Shared variable. Update constraint usage of %p (%g) with variable %p by %g",
               cnst, cnst->usage, elem->variable, elem->variable->mu);
          cnst->usage = MIN(cnst->usage, elem->value * elem->variable->mu);
        }
      }
      if (!cnst->shared) {
        XBT_DEBUG("\tUpdate constraint %p (%g) by %g",
               cnst, cnst->remaining, cnst->usage);

        double_update(&(cnst->remaining), cnst->usage, sg_maxmin_precision);
      }

      XBT_DEBUG("\tRemaining for %p : %g", cnst, cnst->remaining);
      if (cnst->remaining == 0.0) {
        XBT_DEBUG("\tGet rid of constraint %p", cnst);

        xbt_swag_remove(cnst, cnst_list);
        xbt_swag_foreach(_elem, elem_list) {
          elem = (lmm_element_t)_elem;
          if (elem->variable->weight <= 0)
            break;
          if (elem->value > 0) {
            XBT_DEBUG("\t\tGet rid of variable %p", elem->variable);
            xbt_swag_remove(elem->variable, var_list);
          }
        }
      }
    }
  } while (xbt_swag_size(var_list));

  xbt_swag_reset(cnst_list);
  sys->modified = 0;
  if (XBT_LOG_ISENABLED(surf_maxmin, xbt_log_priority_debug)) {
    XBT_DEBUG("Fair bottleneck done");
    lmm_print(sys);
  }
}
