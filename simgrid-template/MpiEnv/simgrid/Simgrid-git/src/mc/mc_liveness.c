/* Copyright (c) 2011-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "mc_private.h"
#include <unistd.h>
#include <sys/wait.h>

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(mc_liveness, mc,
                                "Logging specific to algorithms for liveness properties verification");

/********* Global variables *********/

xbt_dynar_t acceptance_pairs;
xbt_dynar_t successors;
xbt_parmap_t parmap;

/********* Static functions *********/

static xbt_dynar_t get_atomic_propositions_values()
{
  int res;
  int_f_void_t f;
  unsigned int cursor = 0;
  xbt_automaton_propositional_symbol_t ps = NULL;
  xbt_dynar_t values = xbt_dynar_new(sizeof(int), NULL);

  xbt_dynar_foreach(_mc_property_automaton->propositional_symbols, cursor, ps) {
    f = (int_f_void_t) ps->function;
    res = f();
    xbt_dynar_push_as(values, int, res);
  }

  return values;
}


static mc_visited_pair_t is_reached_acceptance_pair(int pair_num,
                                                    xbt_automaton_state_t
                                                    automaton_state,
                                                    xbt_dynar_t
                                                    atomic_propositions)
{

  int raw_mem_set = (mmalloc_get_current_heap() == mc_heap);

  MC_SET_MC_HEAP;

  mc_visited_pair_t pair = NULL;
  pair = MC_visited_pair_new(pair_num, automaton_state, atomic_propositions);
  pair->acceptance_pair = 1;

  if (xbt_dynar_is_empty(acceptance_pairs)) {

    xbt_dynar_push(acceptance_pairs, &pair);

  } else {

    int min = -1, max = -1, index;
    //int res;
    mc_visited_pair_t pair_test;
    int cursor;

    index = get_search_interval(acceptance_pairs, pair, &min, &max);

    if (min != -1 && max != -1) {       // Acceptance pair with same number of processes and same heap bytes used exists

      // Parallell implementation
      /*res = xbt_parmap_mc_apply(parmap, snapshot_compare, xbt_dynar_get_ptr(acceptance_pairs, min), (max-min)+1, pair);
         if(res != -1){
         if(!raw_mem_set)
         MC_SET_STD_HEAP;
         return ((mc_pair_t)xbt_dynar_get_as(acceptance_pairs, (min+res)-1, mc_pair_t))->num;
         } */

      cursor = min;
      while (cursor <= max) {
        pair_test =
            (mc_visited_pair_t) xbt_dynar_get_as(acceptance_pairs, cursor,
                                                 mc_visited_pair_t);
        if (xbt_automaton_state_compare
            (pair_test->automaton_state, pair->automaton_state) == 0) {
          if (xbt_automaton_propositional_symbols_compare_value
              (pair_test->atomic_propositions,
               pair->atomic_propositions) == 0) {
            if (snapshot_compare(pair_test, pair) == 0) {
              XBT_INFO("Pair %d already reached (equal to pair %d) !",
                       pair->num, pair_test->num);

              xbt_fifo_shift(mc_stack);
              if (dot_output != NULL)
                fprintf(dot_output, "\"%d\" -> \"%d\" [%s];\n",
                        initial_global_state->prev_pair, pair_test->num,
                        initial_global_state->prev_req);

              if (!raw_mem_set)
                MC_SET_STD_HEAP;

              return NULL;
            }
          }
        }
        cursor++;
      }
      xbt_dynar_insert_at(acceptance_pairs, min, &pair);
    } else {
      pair_test =
          (mc_visited_pair_t) xbt_dynar_get_as(acceptance_pairs, index,
                                               mc_visited_pair_t);
      if (pair_test->nb_processes < pair->nb_processes) {
        xbt_dynar_insert_at(acceptance_pairs, index + 1, &pair);
      } else {
        if (pair_test->heap_bytes_used < pair->heap_bytes_used)
          xbt_dynar_insert_at(acceptance_pairs, index + 1, &pair);
        else
          xbt_dynar_insert_at(acceptance_pairs, index, &pair);
      }
    }

  }

  if (!raw_mem_set)
    MC_SET_STD_HEAP;

  return pair;

}

static void remove_acceptance_pair(int pair_num)
{

  int raw_mem_set = (mmalloc_get_current_heap() == mc_heap);

  MC_SET_MC_HEAP;

  unsigned int cursor = 0;
  mc_visited_pair_t pair_test = NULL;

  xbt_dynar_foreach(acceptance_pairs, cursor, pair_test) {
    if (pair_test->num == pair_num) {
      break;
    }
  }

  xbt_dynar_remove_at(acceptance_pairs, cursor, &pair_test);

  pair_test->acceptance_removed = 1;

  if (_sg_mc_visited == 0) {
    MC_visited_pair_delete(pair_test);
  } else if (pair_test->visited_removed == 1) {
    MC_visited_pair_delete(pair_test);
  }

  if (!raw_mem_set)
    MC_SET_STD_HEAP;
}


static int MC_automaton_evaluate_label(xbt_automaton_exp_label_t l,
                                       xbt_dynar_t atomic_propositions_values)
{

  switch (l->type) {
  case 0:{
      int left_res =
          MC_automaton_evaluate_label(l->u.or_and.left_exp,
                                      atomic_propositions_values);
      int right_res =
          MC_automaton_evaluate_label(l->u.or_and.right_exp,
                                      atomic_propositions_values);
      return (left_res || right_res);
    }
  case 1:{
      int left_res =
          MC_automaton_evaluate_label(l->u.or_and.left_exp,
                                      atomic_propositions_values);
      int right_res =
          MC_automaton_evaluate_label(l->u.or_and.right_exp,
                                      atomic_propositions_values);
      return (left_res && right_res);
    }
  case 2:{
      int res =
          MC_automaton_evaluate_label(l->u.exp_not, atomic_propositions_values);
      return (!res);
    }
  case 3:{
      unsigned int cursor = 0;
      xbt_automaton_propositional_symbol_t p = NULL;
      xbt_dynar_foreach(_mc_property_automaton->propositional_symbols, cursor,
                        p) {
        if (strcmp(p->pred, l->u.predicat) == 0)
          return (int) xbt_dynar_get_as(atomic_propositions_values, cursor,
                                        int);
      }
      return -1;
    }
  case 4:{
      return 2;
    }
  default:
    return -1;
  }
}

void MC_pre_modelcheck_liveness(void)
{

  initial_global_state->raw_mem_set = (mmalloc_get_current_heap() == mc_heap);

  mc_pair_t initial_pair = NULL;
  smx_process_t process;

  MC_wait_for_requests();

  MC_SET_MC_HEAP;

  acceptance_pairs = xbt_dynar_new(sizeof(mc_visited_pair_t), NULL);
  visited_pairs = xbt_dynar_new(sizeof(mc_visited_pair_t), NULL);
  successors = xbt_dynar_new(sizeof(mc_pair_t), NULL);

  initial_global_state->snapshot = MC_take_snapshot(0);
  initial_global_state->prev_pair = 0;

  MC_SET_STD_HEAP;

  unsigned int cursor = 0;
  xbt_automaton_state_t automaton_state;

  xbt_dynar_foreach(_mc_property_automaton->states, cursor, automaton_state) {
    if (automaton_state->type == -1) {  /* Initial automaton state */

      MC_SET_MC_HEAP;

      initial_pair = MC_pair_new();
      initial_pair->automaton_state = automaton_state;
      initial_pair->graph_state = MC_state_new();
      initial_pair->atomic_propositions = get_atomic_propositions_values();

      /* Get enabled processes and insert them in the interleave set of the graph_state */
      xbt_swag_foreach(process, simix_global->process_list) {
        if (MC_process_is_enabled(process)) {
          MC_state_interleave_process(initial_pair->graph_state, process);
        }
      }

      initial_pair->requests =
          MC_state_interleave_size(initial_pair->graph_state);
      initial_pair->search_cycle = 0;

      xbt_fifo_unshift(mc_stack, initial_pair);
      
      MC_SET_STD_HEAP;

      MC_modelcheck_liveness();

      if (cursor != 0) {
        MC_restore_snapshot(initial_global_state->snapshot);
        MC_SET_STD_HEAP;
      }
    }
  }

  if (initial_global_state->raw_mem_set)
    MC_SET_MC_HEAP;
  else
    MC_SET_STD_HEAP;


}


void MC_modelcheck_liveness()
{

  smx_process_t process;
  mc_pair_t current_pair = NULL;

  if (xbt_fifo_size(mc_stack) == 0)
    return;

  /* Get current pair */
  current_pair =
      (mc_pair_t) xbt_fifo_get_item_content(xbt_fifo_get_first_item(mc_stack));

  /* Update current state in buchi automaton */
  _mc_property_automaton->current_state = current_pair->automaton_state;

  XBT_DEBUG
      ("********************* ( Depth = %d, search_cycle = %d, interleave size %d, pair_num %d)",
       xbt_fifo_size(mc_stack), current_pair->search_cycle,
       MC_state_interleave_size(current_pair->graph_state), current_pair->num);

  mc_stats->visited_pairs++;

  int value;
  smx_simcall_t req = NULL;

  xbt_automaton_transition_t transition_succ;
  unsigned int cursor = 0;
  int res;
  int visited_num;

  mc_pair_t next_pair = NULL;
  xbt_dynar_t prop_values = NULL;
  mc_visited_pair_t reached_pair = NULL;
  int counter_example_depth = 0;

  if (xbt_fifo_size(mc_stack) < _sg_mc_max_depth) {

    if (current_pair->requests > 0) {

      if (current_pair->search_cycle) {

        if ((current_pair->automaton_state->type == 1)
            || (current_pair->automaton_state->type == 2)) {
          if ((reached_pair =
               is_reached_acceptance_pair(current_pair->num,
                                          current_pair->automaton_state,
                                          current_pair->atomic_propositions)) ==
              NULL) {

            counter_example_depth = xbt_fifo_size(mc_stack);
            XBT_INFO("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*");
            XBT_INFO("|             ACCEPTANCE CYCLE            |");
            XBT_INFO("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*");
            XBT_INFO("Counter-example that violates formula :");
            MC_show_stack_liveness(mc_stack);
            MC_dump_stack_liveness(mc_stack);
            MC_print_statistics(mc_stats);
            XBT_INFO("Counter-example depth : %d", counter_example_depth);
            xbt_abort();

          }
        }
      }

      if ((visited_num =
           is_visited_pair(reached_pair, current_pair->num,
                           current_pair->automaton_state,
                           current_pair->atomic_propositions)) != -1) {

        MC_SET_MC_HEAP;
        if (dot_output != NULL)
          fprintf(dot_output, "\"%d\" -> \"%d\" [%s];\n",
                  initial_global_state->prev_pair, visited_num,
                  initial_global_state->prev_req);
        MC_SET_STD_HEAP;

      } else {

        while ((req =
                MC_state_get_request(current_pair->graph_state,
                                     &value)) != NULL) {

          MC_SET_MC_HEAP;
          if (dot_output != NULL) {
            if (initial_global_state->prev_pair != 0
                && initial_global_state->prev_pair != current_pair->num) {
              fprintf(dot_output, "\"%d\" -> \"%d\" [%s];\n",
                      initial_global_state->prev_pair, current_pair->num,
                      initial_global_state->prev_req);
              xbt_free(initial_global_state->prev_req);
            }
            initial_global_state->prev_pair = current_pair->num;
          }
          MC_SET_STD_HEAP;

          MC_LOG_REQUEST(mc_liveness, req, value);

          MC_SET_MC_HEAP;
          if (dot_output != NULL) {
            initial_global_state->prev_req =
                MC_request_get_dot_output(req, value);
            if (current_pair->search_cycle)
              fprintf(dot_output, "%d [shape=doublecircle];\n",
                      current_pair->num);
          }
          MC_SET_STD_HEAP;

          MC_state_set_executed_request(current_pair->graph_state, req, value);
          mc_stats->executed_transitions++;

          /* Answer the request */
          SIMIX_simcall_handle(req, value);

          /* Wait for requests (schedules processes) */
          MC_wait_for_requests();

          MC_SET_MC_HEAP;
          prop_values = get_atomic_propositions_values();
          MC_SET_STD_HEAP;

          int new_pair = 0;

          /* Evaluate enabled transition according to atomic propositions values */
          cursor = 0;
          xbt_dynar_foreach(current_pair->automaton_state->out, cursor,
                            transition_succ) {

            res =
                MC_automaton_evaluate_label(transition_succ->label,
                                            prop_values);

            if (res == 1) {     // enabled transition in automaton

              if (new_pair)
                MC_replay_liveness(mc_stack, 1);

              MC_SET_MC_HEAP;

              next_pair = MC_pair_new();
              next_pair->graph_state = MC_state_new();
              next_pair->automaton_state = transition_succ->dst;
              next_pair->atomic_propositions = get_atomic_propositions_values();

              /* Get enabled processes and insert them in the interleave set of the next graph_state */
              xbt_swag_foreach(process, simix_global->process_list) {
                if (MC_process_is_enabled(process)) {
                  MC_state_interleave_process(next_pair->graph_state, process);
                }
              }

              next_pair->requests =
                  MC_state_interleave_size(next_pair->graph_state);

              if (next_pair->automaton_state->type == 1
                  || next_pair->automaton_state->type == 2
                  || current_pair->search_cycle)
                next_pair->search_cycle = 1;

              xbt_fifo_unshift(mc_stack, next_pair);

              if (mc_stats->expanded_pairs % 1000000 == 0)
                XBT_INFO("Expanded pairs : %lu", mc_stats->expanded_pairs);

              MC_SET_STD_HEAP;

              new_pair = 1;

              MC_modelcheck_liveness();

            }

          }

          /* Then, evaluate true transitions (always true, whatever atomic propositions values) */
          cursor = 0;
          xbt_dynar_foreach(current_pair->automaton_state->out, cursor,
                            transition_succ) {

            res =
                MC_automaton_evaluate_label(transition_succ->label,
                                            prop_values);

            if (res == 2) {     // true transition in automaton

              if (new_pair)
                MC_replay_liveness(mc_stack, 1);

              MC_SET_MC_HEAP;

              next_pair = MC_pair_new();
              next_pair->graph_state = MC_state_new();
              next_pair->automaton_state = transition_succ->dst;
              next_pair->atomic_propositions = get_atomic_propositions_values();

              /* Get enabled process and insert it in the interleave set of the next graph_state */
              xbt_swag_foreach(process, simix_global->process_list) {
                if (MC_process_is_enabled(process)) {
                  MC_state_interleave_process(next_pair->graph_state, process);
                }
              }

              next_pair->requests =
                  MC_state_interleave_size(next_pair->graph_state);

              if (next_pair->automaton_state->type == 1
                  || next_pair->automaton_state->type == 2
                  || current_pair->search_cycle)
                next_pair->search_cycle = 1;

              xbt_fifo_unshift(mc_stack, next_pair);

              if (mc_stats->expanded_pairs % 1000000 == 0)
                XBT_INFO("Expanded pairs : %lu", mc_stats->expanded_pairs);

              MC_SET_STD_HEAP;

              new_pair = 1;

              MC_modelcheck_liveness();

            }

          }

          if (MC_state_interleave_size(current_pair->graph_state) > 0) {
            XBT_DEBUG("Backtracking to depth %d", xbt_fifo_size(mc_stack));
            MC_replay_liveness(mc_stack, 0);
          }

        }

      }

    }

  } else {

    XBT_WARN("/!\\ Max depth reached ! /!\\ ");
    if (MC_state_interleave_size(current_pair->graph_state) > 0) {
      XBT_WARN
          ("/!\\ But, there are still processes to interleave. Model-checker will not be able to ensure the soundness of the verification from now. /!\\ ");
      if (_sg_mc_max_depth == 1000)
        XBT_WARN
            ("Notice : the default value of max depth is 1000 but you can change it with cfg=model-check/max_depth:value.");
    }

  }

  if (xbt_fifo_size(mc_stack) == _sg_mc_max_depth) {
    XBT_DEBUG("Pair %d (depth = %d) shifted in stack, maximum depth reached",
              current_pair->num, xbt_fifo_size(mc_stack));
  } else {
    XBT_DEBUG("Pair %d (depth = %d) shifted in stack", current_pair->num,
              xbt_fifo_size(mc_stack));
  }


  MC_SET_MC_HEAP;
  xbt_dynar_free(&prop_values);
  current_pair = xbt_fifo_shift(mc_stack);
  if (xbt_fifo_size(mc_stack) != _sg_mc_max_depth - 1
      && current_pair->requests > 0 && current_pair->search_cycle) {
    remove_acceptance_pair(current_pair->num);
  }
  MC_pair_delete(current_pair);

  MC_SET_STD_HEAP;

}
