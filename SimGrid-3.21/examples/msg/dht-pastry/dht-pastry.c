/* Copyright (c) 2013-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/msg.h"
#include "xbt/dynar.h"

#include <math.h>
#include <stdio.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(msg_pastry, "Messages specific for this msg example");

/* TODO:                               *
 *  - handle node departure            *
 *  - handle objects on the network    *
 *  - handle neighborhood in the update */

#define COMM_SIZE 10
#define COMP_SIZE 0
#define MAILBOX_NAME_SIZE 10

#define DOMAIN_SIZE 4
#define LEVELS_COUNT 8 // sizeof(int)*8/DOMAIN_SIZE
#define LEVEL_SIZE 16 // 2^DOMAIN_SIZE
#define NEIGHBORHOOD_SIZE 6
#define NAMESPACE_SIZE 6
#define MAILBOX_NAME_SIZE 10

static int nb_bits = 16;
static int timeout = 50;
static int max_simulation_time = 1000;

typedef struct s_node {
  int id;                                 //128bits generated random(2^128 -1)
  int known_id;
  char mailbox[MAILBOX_NAME_SIZE];        // my mailbox name (string representation of the id)
  int namespace_set[NAMESPACE_SIZE];
  int neighborhood_set[NEIGHBORHOOD_SIZE];
  int routing_table[LEVELS_COUNT][LEVEL_SIZE];
  int ready;
  msg_comm_t comm_receive;                // current communication to receive
  xbt_dynar_t pending_tasks;
} s_node_t;
typedef s_node_t* node_t;

typedef struct s_state {
  int id;
  int namespace_set[NAMESPACE_SIZE];
  int neighborhood_set[NEIGHBORHOOD_SIZE];
  int routing_table[LEVELS_COUNT][LEVEL_SIZE];
} s_state_t;
typedef s_state_t* state_t;

/** Types of tasks exchanged between nodes. */
typedef enum {
  TASK_JOIN,
  TASK_JOIN_REPLY,
  TASK_JOIN_LAST_REPLY,
  TASK_UPDATE
} e_task_type_t;

typedef struct s_task_data {
  e_task_type_t type;                     // type of task
  int sender_id;                          // id parameter (used by some types of tasks)
  //int request_finger;                     // finger parameter (used by some types of tasks)
  int answer_id;                          // answer (used by some types of tasks)
  char answer_to[MAILBOX_NAME_SIZE];      // mailbox to send an answer to (if any)
  //const char* issuer_host_name;           // used for logging
  int steps;
  state_t state;
} s_task_data_t;
typedef s_task_data_t* task_data_t;

static int domain(unsigned int a, unsigned int level);
static int shl(int a, int b);
static int closest_in_namespace_set(node_t node, int dest);
static int routing_next(node_t node, int dest);

/**
 * @brief Gets the mailbox name of a host given its chord id.
 * @param node_id id of a node
 * @param mailbox pointer to where the mailbox name should be written
 * (there must be enough space)
 */
static void get_mailbox(int node_id, char* mailbox)
{
  snprintf(mailbox, MAILBOX_NAME_SIZE - 1, "%d", node_id);
}

/** Get the specific level of a node id */
unsigned int domain_mask = 0;
static int domain(unsigned int a, unsigned int level)
{
  if (domain_mask == 0)
    domain_mask = pow(2, DOMAIN_SIZE) - 1;
  unsigned int shift = (LEVELS_COUNT-level-1)*DOMAIN_SIZE;
  return (a >> shift) & domain_mask;
}

/* Get the shared domains between the two givens ids */
static int shl(int a, int b) {
  int l = 0;
  while(l<LEVELS_COUNT && domain(a,l) == domain(b,l))
    l++;
  return l;
}

/* Frees the memory used by a task and destroy it */
static void task_free(void* task)
{
  if(task != NULL){
    s_task_data_t* data = (s_task_data_t*)MSG_task_get_data(task);
    xbt_free(data->state);
    xbt_free(data);
    MSG_task_destroy(task);
  }
}

/* Get the closest id to the dest in the node namespace_set */
static int closest_in_namespace_set(node_t node, int dest) {
  int res = -1;
  if ((node->namespace_set[NAMESPACE_SIZE-1] <= dest) && (dest <= node->namespace_set[0])) {
    int best_dist = abs(node->id - dest);
    res = node->id;
    for (int i=0; i<NAMESPACE_SIZE; i++) {
      if (node->namespace_set[i]!=-1) {
        int dist = abs(node->namespace_set[i] - dest);
        if (dist<best_dist) {
          best_dist = dist;
          res = node->namespace_set[i];
        }
      }
    }
  }
  return res;
}

/* Find the next node to forward a message to */
static int routing_next(node_t node, int dest) {
  int closest = closest_in_namespace_set(node, dest);
  if (closest!=-1)
    return closest;

  int l = shl(node->id, dest);
  int res = node->routing_table[l][domain(dest, l)];
  if (res != -1)
    return res;

  //rare case
  int dist = abs(node->id - dest);
  for (int i=l; i<LEVELS_COUNT; i++) {
    for (int j=0; j<LEVEL_SIZE; j++) {
      res = node->routing_table[i][j];
      if (res!=-1 && abs(res - dest)<dist)
        return res;
    }
  }

  for (int i=0; i<NEIGHBORHOOD_SIZE; i++) {
    res = node->neighborhood_set[i];
    if (res!=-1 && shl(res, dest)>=l && abs(res - dest)<dist)
        return res;
  }

  for (int i=0; i<NAMESPACE_SIZE; i++) {
    res = node->namespace_set[i];
    if (res!=-1 && shl(res, dest)>=l && abs(res - dest)<dist)
        return res;
  }

  return node->id;
}

/* Get the corresponding state of a node */
static state_t node_get_state(node_t node) {
  state_t state = xbt_new0(s_state_t,1);
  state->id = node->id;
  for (int i=0; i<NEIGHBORHOOD_SIZE; i++)
    state->neighborhood_set[i] = node->neighborhood_set[i];

  for (int i=0; i<LEVELS_COUNT; i++)
    for (int j=0; j<LEVEL_SIZE; j++)
      state->routing_table[i][j] = node->routing_table[i][j];

  for (int i=0; i<NAMESPACE_SIZE; i++)
    state->namespace_set[i] = node->namespace_set[i];

  return state;
}

static void print_node_id(node_t node) {
  XBT_INFO(" Id: %i '%08x' ", node->id, (unsigned)node->id);
}

static void print_node_neighborood_set(node_t node) {
  XBT_INFO(" Neighborhood:");
  for (int i=0; i<NEIGHBORHOOD_SIZE; i++)
    XBT_INFO("  %08x", (unsigned)node->neighborhood_set[i]);
}

static void print_node_routing_table(node_t node) {
  XBT_INFO(" Routing table:");
  for (int i=0; i<LEVELS_COUNT; i++){
    for (int j=0; j<LEVEL_SIZE; j++)
      XBT_INFO("  %08x ", (unsigned)node->routing_table[i][j]);
  }
}
/* Print the node namespace set */
static void print_node_namespace_set(node_t node) {
  XBT_INFO(" Namespace:");
  for (int i=0; i<NAMESPACE_SIZE; i++)
    XBT_INFO("  %08x", (unsigned)node->namespace_set[i]);
}

/* Print the node information */
static void print_node(node_t node) {
  XBT_INFO("Node:");
  print_node_id(node);
  print_node_neighborood_set(node);
  print_node_routing_table(node);
  print_node_namespace_set(node);
}

/** Handle a given task */
static void handle_task(node_t node, msg_task_t task) {
  XBT_DEBUG("Handling task %p", task);
  char mailbox[MAILBOX_NAME_SIZE];
  int i;
  int j;
  int min;
  int max;
  int next;
  msg_task_t task_sent;
  task_data_t req_data;
  task_data_t task_data = (task_data_t) MSG_task_get_data(task);
  e_task_type_t type = task_data->type;
  // If the node is not ready keep the task for later
  if (node->ready != 0 && !(type==TASK_JOIN_LAST_REPLY || type==TASK_JOIN_REPLY)) {
    XBT_DEBUG("Task pending %u", type);
    xbt_dynar_push(node->pending_tasks, &task);
    return;
  }
  switch (type) {
    /* Try to join the ring */
    case TASK_JOIN:
      next = routing_next(node, task_data->answer_id);
      XBT_DEBUG("Join request from %08x forwarding to %08x", (unsigned)task_data->answer_id, (unsigned)next);
      type = TASK_JOIN_LAST_REPLY;

      req_data = xbt_new0(s_task_data_t,1);
      req_data->answer_id = task_data->sender_id;
      req_data->steps = task_data->steps + 1;

      // if next different from current node forward the join
      if (next!=node->id) {
        get_mailbox(next, mailbox);
        task_data->sender_id = node->id;
        task_data->steps++;
        task_sent = MSG_task_create(NULL, COMP_SIZE, COMM_SIZE, task_data);
        if (MSG_task_send_with_timeout(task_sent, mailbox, timeout)== MSG_TIMEOUT) {
          XBT_DEBUG("Timeout expired when forwarding join to next %d", next);
          task_free(task_sent);
        }
        type = TASK_JOIN_REPLY;
      }

      // send back the current node state to the joining node
      req_data->type = type;
      req_data->sender_id = node->id;
      get_mailbox(node->id, req_data->answer_to);
      req_data->state = node_get_state(node);
      task_sent = MSG_task_create(NULL, COMP_SIZE, COMM_SIZE, req_data);
      if (MSG_task_send_with_timeout(task_sent, task_data->answer_to, timeout)== MSG_TIMEOUT) {
        XBT_DEBUG("Timeout expired when sending back the current node state to the joining node to %d", node->id);
        task_free(task_sent);
      }
      break;
    /* Join reply from all the node touched by the join  */
    case TASK_JOIN_LAST_REPLY:
      // if last node touched reply, copy its namespace set
      // TODO: it works only if the two nodes are side to side (is it really the case ?)
      j = (task_data->sender_id < node->id) ? -1 : 0;
      for (i=0; i<NAMESPACE_SIZE/2; i++) {
        node->namespace_set[i] = task_data->state->namespace_set[i-j];
        node->namespace_set[NAMESPACE_SIZE-1-i] = task_data->state->namespace_set[NAMESPACE_SIZE-1-i-j-1];
      }
      node->namespace_set[NAMESPACE_SIZE/2+j] = task_data->sender_id;
      node->ready += task_data->steps + 1;
      /* no break */
    case TASK_JOIN_REPLY:
      XBT_DEBUG("Joining Reply");

      // if first node touched reply, copy its neighborhood set
      if (task_data->sender_id == node->known_id) {
        node->neighborhood_set[0] = task_data->sender_id;
        for (i=1; i<NEIGHBORHOOD_SIZE; i++)
          node->neighborhood_set[i] = task_data->state->neighborhood_set[i-1];
      }

      // copy the corresponding routing table levels
      min = (node->id==task_data->answer_id) ? 0 : shl(node->id, task_data->answer_id);
      max = shl(node->id, task_data->sender_id)+1;
      for (i=min;i<max;i++) {
        int d = domain(node->id, i);
        for (j=0; j<LEVEL_SIZE; j++)
          if (d!=j)
            node->routing_table[i][j] =  task_data->state->routing_table[i][j];
      }

      node->ready--;
      // if the node is ready, do all the pending tasks and send update to known nodes
      if (node->ready==0) {
        XBT_DEBUG("Node %i is ready!!!", node->id);
        while (!xbt_dynar_is_empty(node->pending_tasks)) {
          msg_task_t task;
          xbt_dynar_shift(node->pending_tasks, &task);
          handle_task(node, task);
        }

        for (i=0; i<NAMESPACE_SIZE; i++) {
          j = node->namespace_set[i];
          if (j!=-1) {
            XBT_DEBUG("Send update to %i", j);
            get_mailbox(j, mailbox);

            req_data = xbt_new0(s_task_data_t,1);
            req_data->answer_id = node->id;
            req_data->steps = 0;
            req_data->type = TASK_UPDATE;
            req_data->sender_id = node->id;
            get_mailbox(node->id, req_data->answer_to);
            req_data->state = node_get_state(node);
            task_sent = MSG_task_create(NULL, COMP_SIZE, COMM_SIZE, req_data);
            if (MSG_task_send_with_timeout(task_sent, mailbox, timeout)== MSG_TIMEOUT) {
              XBT_DEBUG("Timeout expired when sending update to %d", j);
              task_free(task_sent);
            }
          }
        }
        }
      break;
    /* Received an update of state */
    case TASK_UPDATE:
      XBT_DEBUG("Task update %i !!!", node->id);

      /* Update namespace ses */
      XBT_INFO("Task update from %i !!!", task_data->sender_id);
      XBT_INFO("Node:");
      print_node_id(node);
      print_node_namespace_set(node);
      int curr_namespace_set[NAMESPACE_SIZE];
      int task_namespace_set[NAMESPACE_SIZE+1];

      // Copy the current namespace and the task state namespace with state->id in the middle
      i=0;
      for (; i<NAMESPACE_SIZE/2; i++){
        curr_namespace_set[i] = node->namespace_set[i];
        task_namespace_set[i] = task_data->state->namespace_set[i];
      }
      task_namespace_set[i] = task_data->state->id;
      for (; i<NAMESPACE_SIZE; i++){
        curr_namespace_set[i] = node->namespace_set[i];
        task_namespace_set[i+1] = task_data->state->namespace_set[i];
      }

      // get the index of values before and after node->id in task_namespace
      min = -1;
      max = -1;
      for (i=0; i<=NAMESPACE_SIZE; i++) {
        j = task_namespace_set[i];
        if (j != -1 && j < node->id)
          min = i;
        if (j != -1 && max == -1 && j > node->id)
          max = i;
      }

      // add lower elements
      j = NAMESPACE_SIZE/2-1;
      for (i=NAMESPACE_SIZE/2-1; i>=0; i--) {
        if (min < 0 || curr_namespace_set[j] > task_namespace_set[min]) {
          node->namespace_set[i] = curr_namespace_set[j];
          j--;
        } else if (curr_namespace_set[j] == task_namespace_set[min]) {
          node->namespace_set[i] = curr_namespace_set[j];
          j--;
          min--;
        } else {
          node->namespace_set[i] = task_namespace_set[min];
          min--;
        }
      }

      // add greater elements
      j = NAMESPACE_SIZE/2;
      for (i=NAMESPACE_SIZE/2; i<NAMESPACE_SIZE; i++) {
        if (min<0 || max>=NAMESPACE_SIZE) {
         node->namespace_set[i] = curr_namespace_set[j];
         j++;
        } else if (max >= 0){
          if (curr_namespace_set[j] == -1 || curr_namespace_set[j] > task_namespace_set[max]) {
            node->namespace_set[i] = task_namespace_set[max];
            max++;
          } else if (curr_namespace_set[j] == task_namespace_set[max]) {
            node->namespace_set[i] = curr_namespace_set[j];
            j++;
            max++;
          } else {
            node->namespace_set[i] = curr_namespace_set[j];
            j++;
          }
        }
      }

      /* Update routing table */
      for (i=shl(node->id, task_data->state->id); i<LEVELS_COUNT; i++) {
        for (j=0; j<LEVEL_SIZE; j++) {
          if (node->routing_table[i][j]==-1 && task_data->state->routing_table[i][j]==-1)
            node->routing_table[i][j] = task_data->state->routing_table[i][j];
        }
      }
      break;
    default:
      THROW_IMPOSSIBLE;
  }
  task_free(task);
}

/* Join the ring */
static int join(node_t node){
  task_data_t req_data = xbt_new0(s_task_data_t,1);
  req_data->type = TASK_JOIN;
  req_data->sender_id = node->id;
  req_data->answer_id = node->id;
  req_data->steps = 0;
  get_mailbox(node->id, req_data->answer_to);

  char mailbox[MAILBOX_NAME_SIZE];
  get_mailbox(node->known_id, mailbox);

  msg_task_t task_sent = MSG_task_create(NULL, COMP_SIZE, COMM_SIZE, req_data);
  XBT_DEBUG("Trying to join Pastry ring... (with node %s)", mailbox);
  if (MSG_task_send_with_timeout(task_sent, mailbox, timeout)== MSG_TIMEOUT) {
    XBT_DEBUG("Timeout expired when joining ring with node %d", node->known_id);
    task_free(task_sent);
  }

  return 1;
}

/**
 * @brief Node Function
 * Arguments:
 * - my id
 * - the id of a guy I know in the system (except for the first node)
 * - the time to sleep before I join (except for the first node)
 * - the deadline time
 */
static int node(int argc, char *argv[])
{
  double init_time = MSG_get_clock();
  msg_task_t task_received = NULL;
  int join_success = 0;
  double deadline;
  xbt_assert(argc == 3 || argc == 5, "Wrong number of arguments for this node");
  s_node_t node = {0};
  node.id = xbt_str_parse_int(argv[1], "Invalid ID: %s");
  node.known_id = -1;
  node.ready = -1;
  node.pending_tasks = xbt_dynar_new(sizeof(msg_task_t), NULL);
  get_mailbox(node.id, node.mailbox);
  XBT_DEBUG("New node with id %s (%08x)", node.mailbox, (unsigned)node.id);

  for (int i=0; i<LEVELS_COUNT; i++){
    int d = domain(node.id, i);
    for (int j=0; j<LEVEL_SIZE; j++)
      node.routing_table[i][j] = (d==j) ? node.id : -1;
  }

  for (int i=0; i<NEIGHBORHOOD_SIZE; i++)
    node.neighborhood_set[i] = -1;

  for (int i=0; i<NAMESPACE_SIZE; i++)
    node.namespace_set[i] = -1;

  if (argc == 3) { // first ring
    XBT_DEBUG("Hey! Let's create the system.");
    deadline = xbt_str_parse_double(argv[2], "Invalid deadline: %s");
    node.ready = 0;
    XBT_DEBUG("Create a new Pastry ring...");
    join_success = 1;
  } else {
    node.known_id = xbt_str_parse_int(argv[2], "Invalid known ID: %s");
    double sleep_time = xbt_str_parse_double(argv[3], "Invalid sleep time: %s");
    deadline = xbt_str_parse_double(argv[4], "Invalid deadline: %s");

    // sleep before starting
    XBT_DEBUG("Let's sleep during %f", sleep_time);
    MSG_process_sleep(sleep_time);
    XBT_DEBUG("Hey! Let's join the system.");

    join_success = join(&node);
  }

  if (join_success) {
    XBT_DEBUG("Waiting ….");

    while (MSG_get_clock() < init_time + deadline
//      && MSG_get_clock() < node.last_change_date + 1000
        && MSG_get_clock() < max_simulation_time) {
      if (node.comm_receive == NULL) {
        task_received = NULL;
        node.comm_receive = MSG_task_irecv(&task_received, node.mailbox);
        // FIXME: do not make MSG_task_irecv() calls from several functions
      }
      if (!MSG_comm_test(node.comm_receive)) {
        MSG_process_sleep(5);
      } else {
        // a transfer has occurred

        msg_error_t status = MSG_comm_get_status(node.comm_receive);

        if (status != MSG_OK) {
          XBT_DEBUG("Failed to receive a task. Nevermind.");
          MSG_comm_destroy(node.comm_receive);
          node.comm_receive = NULL;
        } else {
          // the task was successfully received
          MSG_comm_destroy(node.comm_receive);
          node.comm_receive = NULL;
          handle_task(&node, task_received);
        }
      }

    }
  //Cleanup the receiving communication.
  if (node.comm_receive != NULL) {
    if (MSG_comm_test(node.comm_receive) && MSG_comm_get_status(node.comm_receive) == MSG_OK) {
      task_free(MSG_comm_get_task(node.comm_receive));
    }
    MSG_comm_destroy(node.comm_receive);
  }

  }
  xbt_dynar_free(&node.pending_tasks);
  return 1;
}

/** @brief Main function. */
int main(int argc, char *argv[])
{
  MSG_init(&argc, argv);
  xbt_assert(argc > 2,
       "Usage: %s [-nb_bits=n] [-timeout=t] platform_file deployment_file\n"
       "\tExample: %s ../msg_platform.xml pastry10.xml\n",
       argv[0], argv[0]);

  char **options = &argv[1];
  while (!strncmp(options[0], "-", 1)) {
    int length = strlen("-nb_bits=");
    if (!strncmp(options[0], "-nb_bits=", length) && strlen(options[0]) > length) {
      nb_bits = xbt_str_parse_int(options[0] + length, "Invalid nb_bits parameter: %s");
      XBT_DEBUG("Set nb_bits to %d", nb_bits);
    } else {
      length = strlen("-timeout=");
      if (!strncmp(options[0], "-timeout=", length) && strlen(options[0]) > length) {
        timeout = xbt_str_parse_int(options[0] + length, "Invalid timeout parameter: %s");
        XBT_DEBUG("Set timeout to %d", timeout);
      } else {
        xbt_die("Invalid pastry option '%s'", options[0]);
      }
    }
    options++;
  }

  MSG_create_environment(options[0]);

  MSG_function_register("node", node);
  MSG_launch_application(options[1]);

  msg_error_t res = MSG_main();
  XBT_INFO("Simulated time: %g", MSG_get_clock());

  return res != MSG_OK;
}
