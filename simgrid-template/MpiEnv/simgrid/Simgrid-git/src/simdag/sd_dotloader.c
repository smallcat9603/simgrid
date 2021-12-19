/* Copyright (c) 2009-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "private.h"
#include "simdag/simdag.h"
#include "xbt/misc.h"
#include "xbt/log.h"
#include <stdbool.h>
#include <string.h>
#include <libgen.h>

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(sd_dotparse, sd, "Parsing DOT files");

#undef CLEANUP

#ifdef HAVE_CGRAPH_H
#include <graphviz/cgraph.h>
#elif HAVE_AGRAPH_H
#include <graphviz/agraph.h>
#define agnxtnode(dot, node)    agnxtnode(node)
#define agfstin(dot, node)      agfstin(node)
#define agnxtin(dot, edge)      agnxtin(edge)
#define agfstout(dot, node)     agfstout(node)
#define agnxtout(dot, edge)     agnxtout(edge)
#endif

typedef enum {
  sequential =0,
  parallel
} seq_par_t;

xbt_dynar_t SD_dotload_generic(const char * filename, seq_par_t seq_or_par);

static xbt_dynar_t result;
static xbt_dict_t jobs;
static xbt_dict_t computers;
static Agraph_t *dag_dot;
static bool schedule = true;

static void dot_task_p_free(void *task) {
  SD_task_t *t = task;
  SD_task_destroy(*t);
}

#ifdef HAVE_TRACING
static void TRACE_sd_dotloader (SD_task_t task, const char *category) {
  if (category && strlen (category)){
    if (task->category)
      XBT_DEBUG("Change the category of %s from %s to %s",
          task->name, task->category, category);
    else
      XBT_DEBUG("Set the category of %s to %s",task->name, category);
    TRACE_category (category);
    TRACE_sd_set_task_category(task, category);
  }
}
#endif

/** @brief loads a DOT file describing a DAG
 * 
 * See http://www.graphviz.org/doc/info/lang.html
 * for more details.
 * To obtain information about transfers and tasks, two attributes are
 * required : size on task (execution time in Flop) and size on edge
 * (the amount of data transfer in bit).
 * if they aren't here, there choose to be equal to zero.
 */
xbt_dynar_t SD_dotload(const char *filename) {
  computers = xbt_dict_new_homogeneous(NULL);
  schedule = false;
  SD_dotload_generic(filename, sequential);
  xbt_dynar_t computer = NULL;
  xbt_dict_cursor_t dict_cursor;
  char *computer_name;
  xbt_dict_foreach(computers,dict_cursor,computer_name,computer){
    xbt_dynar_free(&computer);
  }
  xbt_dict_free(&computers);
  return result;
}

xbt_dynar_t SD_dotload_with_sched(const char *filename) {
  computers = xbt_dict_new_homogeneous(NULL);
  SD_dotload_generic(filename, sequential);

  if(schedule){
    xbt_dynar_t computer = NULL;
    xbt_dict_cursor_t dict_cursor;
    char *computer_name;
    const SD_workstation_t *workstations = SD_workstation_get_list ();
    xbt_dict_foreach(computers,dict_cursor,computer_name,computer){
      int count_computer = atoi(computer_name);
      unsigned int count=0;
      SD_task_t task;
      SD_task_t task_previous = NULL;
      xbt_dynar_foreach(computer,count,task){
        /* add dependency between the previous and the task to avoid
         * parallel execution */
        if(task != NULL ){
          if(task_previous != NULL &&
             !SD_task_dependency_exists(task_previous, task))
            SD_task_dependency_add(NULL, NULL, task_previous, task);
          SD_task_schedulel(task, 1, workstations[count_computer]);
          task_previous = task;
        }
      }
      xbt_dynar_free(&computer);
    }
    xbt_dict_free(&computers);
    if(acyclic_graph_detail(result))
      return result;
    else
      XBT_WARN("There is at least one cycle in the provided task graph");
  }else{
    XBT_WARN("The scheduling is ignored");
  }
  xbt_dynar_t computer = NULL;
  xbt_dict_cursor_t dict_cursor;
  char *computer_name;
  xbt_dict_foreach(computers,dict_cursor,computer_name,computer){
    xbt_dynar_free(&computer);
  }
  xbt_dict_free(&computers);
  xbt_dynar_free(&result);
  return NULL;
}

xbt_dynar_t SD_PTG_dotload(const char * filename) {
  xbt_dynar_t result = SD_dotload_generic(filename, parallel);
  if (!acyclic_graph_detail(result)) {
    XBT_ERROR("The DOT described in %s is not a DAG. It contains a cycle.",
              basename((char*)filename));
    xbt_dynar_free(&result);
    /* (result == NULL) here */
  }
  return result;
}

#ifdef HAVE_CGRAPH_H
static int edge_compare(const void *a, const void *b)
{
  unsigned va = AGSEQ(*(Agedge_t **)a);
  unsigned vb = AGSEQ(*(Agedge_t **)b);
  return va == vb ? 0 : (va < vb ? -1 : 1);
}
#endif

xbt_dynar_t SD_dotload_generic(const char * filename, seq_par_t seq_or_par){
  xbt_assert(filename, "Unable to use a null file descriptor\n");
  unsigned int i;
  result = xbt_dynar_new(sizeof(SD_task_t), dot_task_p_free);
  jobs = xbt_dict_new_homogeneous(NULL);
  FILE *in_file = fopen(filename, "r");
  if (in_file == NULL)
    xbt_die("Failed to open file: %s", filename);
  dag_dot = agread(in_file, NIL(Agdisc_t *));
  SD_task_t root, end, task;
  /*
   * Create all the nodes
   */
  Agnode_t *node = NULL;
  for (node = agfstnode(dag_dot); node; node = agnxtnode(dag_dot, node)) {

    char *name = agnameof(node);
    double amount = atof(agget(node, (char *) "size"));
    double alpha = 0.0;

    if (seq_or_par == sequential){
      XBT_DEBUG("See <job id=%s amount =%.0f>", name, amount);
    } else {
      if (!strcmp(agget(node, (char *) "alpha"), "")){
        alpha = atof(agget(node, (char *) "alpha"));
        if (alpha == -1.){
          XBT_DEBUG("negative alpha value provided. Set to 0.");
          alpha = 0.0 ;
        }
      } else {
        XBT_DEBUG("no alpha value provided. Set to 0");
        alpha = 0.0 ;
      }

      XBT_DEBUG("See <job id=%s amount =%.0f alpha = %.3f>",
          name, amount, alpha);
    }

    if (!(task = xbt_dict_get_or_null(jobs, name))) {
      if (seq_or_par == sequential){
        task = SD_task_create_comp_seq(name, NULL , amount);
      } else {
        task = SD_task_create_comp_par_amdahl(name, NULL , amount, alpha);
      }
#ifdef HAVE_TRACING
      TRACE_sd_dotloader (task, agget (node, (char*)"category"));
#endif
      xbt_dict_set(jobs, name, task, NULL);
      if (!strcmp(name, "root")){
      /* by design the root task is always SCHEDULABLE */
      __SD_task_set_state(task, SD_SCHEDULABLE);
      /* Put it at the beginning of the dynar */
        xbt_dynar_insert_at(result, 0, &task);
      } else {
        if (!strcmp(name, "end")){
          XBT_DEBUG("Declaration of the 'end' node, don't store it yet.");
          end = task;
          /* Should be inserted later in the dynar */
        } else {
          xbt_dynar_push(result, &task);
        }
      }

      if((seq_or_par == sequential) &&
          (schedule ||
              XBT_LOG_ISENABLED(sd_dotparse, xbt_log_priority_verbose))){
        /* try to take the information to schedule the task only if all is
         * right*/
        int performer, order;
        char *char_performer = agget(node, (char *) "performer");
        char *char_order = agget(node, (char *) "order");
        /* performer is the computer which execute the task */
        performer =
            ((!char_performer || !strcmp(char_performer,"")) ? -1:atoi(char_performer));
        /* order is giving the task order on one computer */
        order = ((!char_order || !strcmp(char_order, ""))? -1:atoi(char_order));

        XBT_DEBUG ("Task '%s' is scheduled on workstation '%d' in position '%d'",
                    task->name, performer, order);
        xbt_dynar_t computer = NULL;
        if(performer != -1 && order != -1){
          /* required parameters are given */
          computer = xbt_dict_get_or_null(computers, char_performer);
          if(computer == NULL){
            computer = xbt_dynar_new(sizeof(SD_task_t), NULL);
            xbt_dict_set(computers, char_performer, computer, NULL);
          }
          if(performer < xbt_lib_length(host_lib)){
            /* the wanted computer is available */
            SD_task_t *task_test = NULL;
            if(order < computer->used)
              task_test = xbt_dynar_get_ptr(computer,order);
            if(task_test != NULL && *task_test != NULL && *task_test != task){
              /* the user gives the same order to several tasks */
              schedule = false;
              XBT_VERB("The task %s starts on the computer %s at the position : %s like the task %s",
                     (*task_test)->name, char_performer, char_order,
                     task->name);
            }else{
              /* the parameter seems to be ok */
              xbt_dynar_set_as(computer, order, SD_task_t, task);
            }
          }else{
            /* the platform has not enough processors to schedule the DAG like
             * the user wants*/
            schedule = false;
            XBT_VERB("The schedule is ignored, there are not enough computers");
          }
        }
        else {
          /* one of required parameters is not given */
          schedule = false;
          XBT_VERB("The schedule is ignored, the task %s is not correctly scheduled",
              task->name);
        }
      }
    } else {
      XBT_WARN("Task '%s' is defined more than once", name);
    }
  }

  /*
   * Check if 'root' and 'end' nodes have been explicitly declared.
   * If not, create them.
   */
  if (!(root = xbt_dict_get_or_null(jobs, "root"))){
    if (seq_or_par == sequential)
      root = SD_task_create_comp_seq("root", NULL, 0);
    else
      root = SD_task_create_comp_par_amdahl("root", NULL, 0, 0);
    /* by design the root task is always SCHEDULABLE */
    __SD_task_set_state(root, SD_SCHEDULABLE);
    /* Put it at the beginning of the dynar */
      xbt_dynar_insert_at(result, 0, &root);
  }

  if (!(end = xbt_dict_get_or_null(jobs, "end"))){
    if (seq_or_par == sequential)
      end = SD_task_create_comp_seq("end", NULL, 0);
    else
      end = SD_task_create_comp_par_amdahl("end", NULL, 0, 0);
    /* Should be inserted later in the dynar */
  }

  /*
   * Create edges
   */
  xbt_dynar_t edges = xbt_dynar_new(sizeof(Agedge_t*), NULL);
  for (node = agfstnode(dag_dot); node; node = agnxtnode(dag_dot, node)) {
    unsigned cursor;
    Agedge_t * edge;
    xbt_dynar_reset(edges);
    for (edge = agfstout(dag_dot, node); edge; edge = agnxtout(dag_dot, edge))
      xbt_dynar_push_as(edges, Agedge_t *, edge);
#ifdef HAVE_CGRAPH_H
    /* Hack: circumvent a bug in libcgraph, where the edges are not always given
     * back in creation order.  We sort them again, according to their sequence
     * id.  The problem appears to be solved (i.e.: I did not test it) in
     * graphviz' mercurial repository by the following changeset:
     *    changeset:   8431:d5f1fb7e8103
     *    user:        Emden Gansner <erg@research.att.com>
     *    date:        Tue Oct 11 12:38:58 2011 -0400
     *    summary:     Make sure edges are stored in node creation order
     * It should be fixed in graphviz 2.30 and above.
     */
    xbt_dynar_sort(edges, edge_compare);
#endif
    xbt_dynar_foreach(edges, cursor, edge) {
      SD_task_t src, dst;
      char *src_name=agnameof(agtail(edge));
      char *dst_name=agnameof(aghead(edge));
      double size = atof(agget(edge, (char *) "size"));

      src = xbt_dict_get_or_null(jobs, src_name);
      dst  = xbt_dict_get_or_null(jobs, dst_name);

      if (size > 0) {
        char *name =
            xbt_malloc((strlen(src_name)+strlen(dst_name)+6)*sizeof(char));
        sprintf(name, "%s->%s", src_name, dst_name);
        XBT_DEBUG("See <transfer id=%s amount = %.0f>", name, size);
        if (!(task = xbt_dict_get_or_null(jobs, name))) {
          if (seq_or_par == sequential)
            task = SD_task_create_comm_e2e(name, NULL , size);
          else
            task = SD_task_create_comm_par_mxn_1d_block(name, NULL , size);
#ifdef HAVE_TRACING
          TRACE_sd_dotloader (task, agget (node, (char*)"category"));
#endif
          SD_task_dependency_add(NULL, NULL, src, task);
          SD_task_dependency_add(NULL, NULL, task, dst);
          xbt_dict_set(jobs, name, task, NULL);
          xbt_dynar_push(result, &task);
        } else {
          XBT_WARN("Task '%s' is defined more than once", name);
        }
        xbt_free(name);
      } else {
        SD_task_dependency_add(NULL, NULL, src, dst);
      }
    }
  }
  xbt_dynar_free(&edges);

  /* all compute and transfer tasks have been created, put the "end" node at
   * the end of dynar
   */
  XBT_DEBUG("All tasks have been created, put %s at the end of the dynar",
      end->name);
  xbt_dynar_push(result, &end);

  /* Connect entry tasks to 'root', and exit tasks to 'end'*/

  xbt_dynar_foreach (result, i, task){
    if (task == root || task == end)
      continue;
    if (xbt_dynar_is_empty(task->tasks_before)) {
      XBT_DEBUG("file '%s' has no source. Add dependency from 'root'",
          task->name);
      SD_task_dependency_add(NULL, NULL, root, task);
    } else if (xbt_dynar_is_empty(task->tasks_after)) {
      XBT_DEBUG("file '%s' has no destination. Add dependency to 'end'",
          task->name);
      SD_task_dependency_add(NULL, NULL, task, end);
    }
  }

  agclose(dag_dot);
  xbt_dict_free(&jobs);
  fclose(in_file);

  if (!acyclic_graph_detail(result)) {
    XBT_ERROR("The DOT described in %s is not a DAG. It contains a cycle.",
              basename((char*)filename));
    xbt_dynar_free(&result);
    /* (result == NULL) here */
  }
  return result;
}
