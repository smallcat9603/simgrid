/* Copyright (c) 2011-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "mc_private.h"
#include <unistd.h>
#include <sys/wait.h>

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(mc_visited, mc,
                                "Logging specific to state equaity detection mechanisms");

xbt_dynar_t visited_pairs;
xbt_dynar_t visited_states;

void visited_state_free(mc_visited_state_t state)
{
  if (state) {
    MC_free_snapshot(state->system_state);
    xbt_free(state);
  }
}

void visited_state_free_voidp(void *s)
{
  visited_state_free((mc_visited_state_t) * (void **) s);
}

/**
 * \brief Save the current state
 * \return Snapshot of the current state.
 */
static mc_visited_state_t visited_state_new()
{
  mc_visited_state_t new_state = NULL;
  new_state = xbt_new0(s_mc_visited_state_t, 1);
  new_state->heap_bytes_used = mmalloc_get_bytes_used(std_heap);
  new_state->nb_processes = xbt_swag_size(simix_global->process_list);
  new_state->system_state = MC_take_snapshot(mc_stats->expanded_states);
  new_state->num = mc_stats->expanded_states;
  new_state->other_num = -1;
  return new_state;
}


mc_visited_pair_t MC_visited_pair_new(int pair_num,
                                      xbt_automaton_state_t automaton_state,
                                      xbt_dynar_t atomic_propositions)
{
  mc_visited_pair_t pair = NULL;
  pair = xbt_new0(s_mc_visited_pair_t, 1);
  pair->graph_state = MC_state_new();
  pair->graph_state->system_state = MC_take_snapshot(pair_num);
  pair->heap_bytes_used = mmalloc_get_bytes_used(std_heap);
  pair->nb_processes = xbt_swag_size(simix_global->process_list);
  pair->automaton_state = automaton_state;
  pair->num = pair_num;
  pair->other_num = -1;
  pair->acceptance_removed = 0;
  pair->visited_removed = 0;
  pair->acceptance_pair = 0;
  pair->atomic_propositions = xbt_dynar_new(sizeof(int), NULL);
  unsigned int cursor = 0;
  int value;
  xbt_dynar_foreach(atomic_propositions, cursor, value)
      xbt_dynar_push_as(pair->atomic_propositions, int, value);
  return pair;
}

void MC_visited_pair_delete(mc_visited_pair_t p)
{
  p->automaton_state = NULL;
  MC_state_delete(p->graph_state);
  xbt_dynar_free(&(p->atomic_propositions));
  xbt_free(p);
  p = NULL;
}

/**
 *  \brief Find a suitable subrange of candidate duplicates for a given state
 *  \param list dynamic array of states/pairs with candidate duplicates of the current state;
 *  \param ref current state/pair;
 *  \param min (output) index of the beginning of the the subrange
 *  \param max (output) index of the enf of the subrange
 *
 *  Given a suitably ordered array of states/pairs, this function extracts a subrange
 *  (with index *min <= i <= *max) with candidate duplicates of the given state/pair.
 *  This function uses only fast discriminating criterions and does not use the
 *  full state/pair comparison algorithms.
 *
 *  The states/pairs in list MUST be ordered using a (given) weak order
 *  (based on nb_processes and heap_bytes_used).
 *  The subrange is the subrange of "equivalence" of the given state/pair.
 */
int get_search_interval(xbt_dynar_t list, void *ref, int *min, int *max)
{

  int mc_mem_set = (mmalloc_get_current_heap() == mc_heap);

  MC_SET_MC_HEAP;

  int cursor = 0, previous_cursor, next_cursor;
  int nb_processes, heap_bytes_used, nb_processes_test, heap_bytes_used_test;
  void *ref_test;

  if (_sg_mc_liveness) {
    nb_processes = ((mc_visited_pair_t) ref)->nb_processes;
    heap_bytes_used = ((mc_visited_pair_t) ref)->heap_bytes_used;
  } else {
    nb_processes = ((mc_visited_state_t) ref)->nb_processes;
    heap_bytes_used = ((mc_visited_state_t) ref)->heap_bytes_used;
  }

  int start = 0;
  int end = xbt_dynar_length(list) - 1;

  while (start <= end) {
    cursor = (start + end) / 2;
    if (_sg_mc_liveness) {
      ref_test =
        (mc_visited_pair_t) xbt_dynar_get_as(list, cursor, mc_visited_pair_t);
      nb_processes_test = ((mc_visited_pair_t) ref_test)->nb_processes;
      heap_bytes_used_test = ((mc_visited_pair_t) ref_test)->heap_bytes_used;
    } else {
      ref_test =
        (mc_visited_state_t) xbt_dynar_get_as(list, cursor,
                                              mc_visited_state_t);
      nb_processes_test = ((mc_visited_state_t) ref_test)->nb_processes;
      heap_bytes_used_test = ((mc_visited_state_t) ref_test)->heap_bytes_used;
    }
    if (nb_processes_test < nb_processes) {
      start = cursor + 1;
    } else if (nb_processes_test > nb_processes) {
      end = cursor - 1;
    } else {
      if (heap_bytes_used_test < heap_bytes_used) {
        start = cursor + 1;
      } else if (heap_bytes_used_test > heap_bytes_used) {
        end = cursor - 1;
      } else {
        *min = *max = cursor;
        previous_cursor = cursor - 1;
        while (previous_cursor >= 0) {
          if (_sg_mc_liveness) {
            ref_test =
              (mc_visited_pair_t) xbt_dynar_get_as(list, previous_cursor,
                                                   mc_visited_pair_t);
            nb_processes_test = ((mc_visited_pair_t) ref_test)->nb_processes;
            heap_bytes_used_test =
              ((mc_visited_pair_t) ref_test)->heap_bytes_used;
          } else {
            ref_test =
                (mc_visited_state_t) xbt_dynar_get_as(list, previous_cursor,
                                                      mc_visited_state_t);
            nb_processes_test = ((mc_visited_state_t) ref_test)->nb_processes;
            heap_bytes_used_test =
                ((mc_visited_state_t) ref_test)->heap_bytes_used;
          }
          if (nb_processes_test != nb_processes
              || heap_bytes_used_test != heap_bytes_used)
            break;
          *min = previous_cursor;
          previous_cursor--;
        }
        next_cursor = cursor + 1;
        while (next_cursor < xbt_dynar_length(list)) {
          if (_sg_mc_liveness) {
            ref_test =
                (mc_visited_pair_t) xbt_dynar_get_as(list, next_cursor,
                                                     mc_visited_pair_t);
            nb_processes_test = ((mc_visited_pair_t) ref_test)->nb_processes;
            heap_bytes_used_test =
                ((mc_visited_pair_t) ref_test)->heap_bytes_used;
          } else {
            ref_test =
              (mc_visited_state_t) xbt_dynar_get_as(list, next_cursor,
                                                    mc_visited_state_t);
            nb_processes_test = ((mc_visited_state_t) ref_test)->nb_processes;
            heap_bytes_used_test =
              ((mc_visited_state_t) ref_test)->heap_bytes_used;
          }
          if (nb_processes_test != nb_processes
              || heap_bytes_used_test != heap_bytes_used)
            break;
          *max = next_cursor;
          next_cursor++;
        }
        if (!mc_mem_set)
          MC_SET_STD_HEAP;
        return -1;
      }
    }
  }

  if (!mc_mem_set)
    MC_SET_STD_HEAP;

  return cursor;
}


/**
 * \brief Checks whether a given state has already been visited by the algorithm.
 */

mc_visited_state_t is_visited_state()
{

  if (_sg_mc_visited == 0)
    return NULL;

  /* If comm determinism verification, we cannot stop the exploration if some 
     communications are not finished (at least, data are transfered). These communications 
     are incomplete and they cannot be analyzed and compared with the initial pattern */
  if (_sg_mc_comms_determinism || _sg_mc_send_determinism) {
    int current_process = 1;
    while (current_process < simix_process_maxpid) {
      if (!xbt_dynar_is_empty((xbt_dynar_t)xbt_dynar_get_as(incomplete_communications_pattern, current_process, xbt_dynar_t)))
        return NULL;
      current_process++;
    }
  }

  int mc_mem_set = (mmalloc_get_current_heap() == mc_heap);

  MC_SET_MC_HEAP;

  mc_visited_state_t new_state = visited_state_new();

  if (xbt_dynar_is_empty(visited_states)) {

    xbt_dynar_push(visited_states, &new_state);

    if (!mc_mem_set)
      MC_SET_STD_HEAP;

    return NULL;

  } else {

    int min = -1, max = -1, index;
    //int res;
    mc_visited_state_t state_test;
    int cursor;

    index = get_search_interval(visited_states, new_state, &min, &max);

    if (min != -1 && max != -1) {

      // Parallell implementation
      /*res = xbt_parmap_mc_apply(parmap, snapshot_compare, xbt_dynar_get_ptr(visited_states, min), (max-min)+1, new_state);
         if(res != -1){
         state_test = (mc_visited_state_t)xbt_dynar_get_as(visited_states, (min+res)-1, mc_visited_state_t);
         if(state_test->other_num == -1)
         new_state->other_num = state_test->num;
         else
         new_state->other_num = state_test->other_num;
         if(dot_output == NULL)
         XBT_DEBUG("State %d already visited ! (equal to state %d)", new_state->num, state_test->num);
         else
         XBT_DEBUG("State %d already visited ! (equal to state %d (state %d in dot_output))", new_state->num, state_test->num, new_state->other_num);
         xbt_dynar_remove_at(visited_states, (min + res) - 1, NULL);
         xbt_dynar_insert_at(visited_states, (min+res) - 1, &new_state);
         if(!raw_mem_set)
         MC_SET_STD_HEAP;
         return new_state->other_num;
         } */

      cursor = min;
      while (cursor <= max) {
        state_test =
            (mc_visited_state_t) xbt_dynar_get_as(visited_states, cursor,
                                                  mc_visited_state_t);
        if (snapshot_compare(state_test, new_state) == 0) {
          // The state has been visited:

          if (state_test->other_num == -1)
            new_state->other_num = state_test->num;
          else
            new_state->other_num = state_test->other_num;
          if (dot_output == NULL)
            XBT_DEBUG("State %d already visited ! (equal to state %d)",
                      new_state->num, state_test->num);
          else
            XBT_DEBUG
                ("State %d already visited ! (equal to state %d (state %d in dot_output))",
                 new_state->num, state_test->num, new_state->other_num);

          /* Replace the old state with the new one (with a bigger num) 
             (when the max number of visited states is reached,  the oldest 
             one is removed according to its number (= with the min number) */
          xbt_dynar_remove_at(visited_states, cursor, NULL);
          xbt_dynar_insert_at(visited_states, cursor, &new_state);

          if (!mc_mem_set)
            MC_SET_STD_HEAP;
          return state_test;
        }
        cursor++;
      }

      // The state has not been visited: insert the state in the dynamic array.
      xbt_dynar_insert_at(visited_states, min, &new_state);

    } else {

      // The state has not been visited: insert the state in the dynamic array.
      state_test =
          (mc_visited_state_t) xbt_dynar_get_as(visited_states, index,
                                                mc_visited_state_t);
      if (state_test->nb_processes < new_state->nb_processes) {
        xbt_dynar_insert_at(visited_states, index + 1, &new_state);
      } else {
        if (state_test->heap_bytes_used < new_state->heap_bytes_used)
          xbt_dynar_insert_at(visited_states, index + 1, &new_state);
        else
          xbt_dynar_insert_at(visited_states, index, &new_state);
      }

    }

    // We have reached the maximum number of stored states;
    if (xbt_dynar_length(visited_states) > _sg_mc_visited) {

      // Find the (index of the) older state (with the smallest num):
      int min2 = mc_stats->expanded_states;
      unsigned int cursor2 = 0;
      unsigned int index2 = 0;
      xbt_dynar_foreach(visited_states, cursor2, state_test){
        if (!mc_important_snapshot(state_test->system_state) && state_test->num < min2) {
          index2 = cursor2;
          min2 = state_test->num;
        }
      }

      // and drop it:
      xbt_dynar_remove_at(visited_states, index2, NULL);
    }

    if (!mc_mem_set)
      MC_SET_STD_HEAP;

    return NULL;

  }
}

/**
 * \brief Checks whether a given pair has already been visited by the algorithm.
 */
int is_visited_pair(mc_visited_pair_t pair, int pair_num,
                    xbt_automaton_state_t automaton_state,
                    xbt_dynar_t atomic_propositions)
{

  if (_sg_mc_visited == 0)
    return -1;

  int mc_mem_set = (mmalloc_get_current_heap() == mc_heap);

  MC_SET_MC_HEAP;

  mc_visited_pair_t new_pair = NULL;

  if (pair == NULL) {
    new_pair =
        MC_visited_pair_new(pair_num, automaton_state, atomic_propositions);
  } else {
    new_pair = pair;
  }

  if (xbt_dynar_is_empty(visited_pairs)) {

    xbt_dynar_push(visited_pairs, &new_pair);

  } else {

    int min = -1, max = -1, index;
    //int res;
    mc_visited_pair_t pair_test;
    int cursor;

    index = get_search_interval(visited_pairs, new_pair, &min, &max);

    if (min != -1 && max != -1) {       // Visited pair with same number of processes and same heap bytes used exists
      /*res = xbt_parmap_mc_apply(parmap, snapshot_compare, xbt_dynar_get_ptr(visited_pairs, min), (max-min)+1, pair);
         if(res != -1){
         pair_test = (mc_pair_t)xbt_dynar_get_as(visited_pairs, (min+res)-1, mc_pair_t);
         if(pair_test->other_num == -1)
         pair->other_num = pair_test->num;
         else
         pair->other_num = pair_test->other_num;
         if(dot_output == NULL)
         XBT_DEBUG("Pair %d already visited ! (equal to pair %d)", pair->num, pair_test->num);
         else
         XBT_DEBUG("Pair %d already visited ! (equal to pair %d (pair %d in dot_output))", pair->num, pair_test->num, pair->other_num);
         xbt_dynar_remove_at(visited_pairs, (min + res) - 1, NULL);
         xbt_dynar_insert_at(visited_pairs, (min+res) - 1, &pair);
         pair_test->visited_removed = 1;
         if(pair_test->stack_removed && pair_test->visited_removed){
         if((pair_test->automaton_state->type == 1) || (pair_test->automaton_state->type == 2)){
         if(pair_test->acceptance_removed){
         MC_pair_delete(pair_test);
         }
         }else{
         MC_pair_delete(pair_test);
         }
         }
         if(!raw_mem_set)
         MC_SET_STD_HEAP;
         return pair->other_num;
         } */
      cursor = min;
      while (cursor <= max) {
        pair_test =
            (mc_visited_pair_t) xbt_dynar_get_as(visited_pairs, cursor,
                                                 mc_visited_pair_t);
        if (xbt_automaton_state_compare
            (pair_test->automaton_state, new_pair->automaton_state) == 0) {
          if (xbt_automaton_propositional_symbols_compare_value
              (pair_test->atomic_propositions,
               new_pair->atomic_propositions) == 0) {
            if (snapshot_compare(pair_test, new_pair) == 0) {
              if (pair_test->other_num == -1)
                new_pair->other_num = pair_test->num;
              else
                new_pair->other_num = pair_test->other_num;
              if (dot_output == NULL)
                XBT_DEBUG("Pair %d already visited ! (equal to pair %d)",
                          new_pair->num, pair_test->num);
              else
                XBT_DEBUG
                    ("Pair %d already visited ! (equal to pair %d (pair %d in dot_output))",
                     new_pair->num, pair_test->num, new_pair->other_num);
              xbt_dynar_remove_at(visited_pairs, cursor, NULL);
              xbt_dynar_insert_at(visited_pairs, cursor, &new_pair);
              pair_test->visited_removed = 1;
              if (pair_test->acceptance_pair) {
                if (pair_test->acceptance_removed == 1)
                  MC_visited_pair_delete(pair_test);
              } else {
                MC_visited_pair_delete(pair_test);
              }
              if (!mc_mem_set)
                MC_SET_STD_HEAP;
              return new_pair->other_num;
            }
          }
        }
        cursor++;
      }
      xbt_dynar_insert_at(visited_pairs, min, &new_pair);
    } else {
      pair_test =
          (mc_visited_pair_t) xbt_dynar_get_as(visited_pairs, index,
                                               mc_visited_pair_t);
      if (pair_test->nb_processes < new_pair->nb_processes) {
        xbt_dynar_insert_at(visited_pairs, index + 1, &new_pair);
      } else {
        if (pair_test->heap_bytes_used < new_pair->heap_bytes_used)
          xbt_dynar_insert_at(visited_pairs, index + 1, &new_pair);
        else
          xbt_dynar_insert_at(visited_pairs, index, &new_pair);
      }
    }

    if (xbt_dynar_length(visited_pairs) > _sg_mc_visited) {
      int min2 = mc_stats->expanded_pairs;
      unsigned int cursor2 = 0;
      unsigned int index2 = 0;
      xbt_dynar_foreach(visited_pairs, cursor2, pair_test) {
        if (!mc_important_snapshot(pair_test->graph_state->system_state) && pair_test->num < min2) {
          index2 = cursor2;
          min2 = pair_test->num;
        }
      }
      xbt_dynar_remove_at(visited_pairs, index2, &pair_test);
      pair_test->visited_removed = 1;
      if (pair_test->acceptance_pair) {
        if (pair_test->acceptance_removed)
          MC_visited_pair_delete(pair_test);
      } else {
        MC_visited_pair_delete(pair_test);
      }
    }

  }

  if (!mc_mem_set)
    MC_SET_STD_HEAP;

  return -1;
}
