/* Copyright (c) 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "tracker.h"
#include <msg/msg.h>
#include <xbt/RngStream.h>

static void task_free(void *data);

XBT_LOG_NEW_DEFAULT_CATEGORY(msg_tracker, "Messages specific for the tracker");
/**
 * Tracker main function
 * @param argc number of arguments
 * @param argv arguments
 */
int tracker(int argc, char *argv[])
{
  int i;

  RngStream stream = (RngStream) MSG_host_get_property_value(MSG_host_self(), "stream");
  //Checking arguments
  xbt_assert(argc == 2, "Wrong number of arguments for the tracker.");
  //Retrieving end time
  double deadline = atof(argv[1]);
  xbt_assert(deadline > 0, "Wrong deadline supplied");
  //Building peers array
  xbt_dynar_t peers_list = xbt_dynar_new(sizeof(int), NULL);

  XBT_INFO("Tracker launched.");

  msg_comm_t comm_received = NULL;
  msg_task_t task_received = NULL;

  while (MSG_get_clock() < deadline) {
    if (comm_received == NULL) {
      comm_received = MSG_task_irecv(&task_received, TRACKER_MAILBOX);
    }
    if (MSG_comm_test(comm_received)) {
      //Check for correct status
      if (MSG_comm_get_status(comm_received) == MSG_OK) {
        //Retrieve the data sent by the peer.
        tracker_task_data_t data = MSG_task_get_data(task_received);
        //Add the peer to our peer list.
        if (!is_in_list(peers_list, data->peer_id)) {
          xbt_dynar_push_as(peers_list, int, data->peer_id);
        }
        //Sending peers to the peer
        int next_peer;
        int peers_length = xbt_dynar_length(peers_list);
        for (i = 0; i < MAXIMUM_PAIRS && i < peers_length; i++) {
          do {
            next_peer =
                xbt_dynar_get_as(peers_list,
                                 RngStream_RandInt(stream, 0, peers_length - 1),
                                 int);
          } while (is_in_list(data->peers, next_peer));
          xbt_dynar_push_as(data->peers, int, next_peer);
        }
        //setting the interval
        data->interval = TRACKER_QUERY_INTERVAL;
        //sending the task back to the peer.
        MSG_task_dsend(task_received, data->mailbox, task_free);
        //destroy the communication.
      }
      MSG_comm_destroy(comm_received);
      comm_received = NULL;
      task_received = NULL;
    } else {
      MSG_process_sleep(1);
    }
  }
  //Free the remaining communication if any
  if (comm_received) {
    MSG_comm_destroy(comm_received);
  }
  //Free the peers list
  xbt_dynar_free(&peers_list);

  XBT_INFO("Tracker is leaving");

  return 0;
}

/**
 * Build a new task for the tracker.
 * @param issuer_host_name Hostname of the issuer. For debugging purposes
 */
tracker_task_data_t tracker_task_data_new(const char *issuer_host_name,
                                          const char *mailbox, int peer_id,
                                          int uploaded, int downloaded,
                                          int left)
{
  tracker_task_data_t task = xbt_new(s_tracker_task_data_t, 1);

  task->type = TRACKER_TASK_QUERY;
  task->issuer_host_name = issuer_host_name;
  task->mailbox = mailbox;
  task->peer_id = peer_id;
  task->uploaded = uploaded;
  task->downloaded = downloaded;
  task->left = left;

  task->peers = xbt_dynar_new(sizeof(int), NULL);

  return task;
}

/**
 * Free a tracker task that has not succefully been sent.
 * @param data Task to free
 */
static void task_free(void *data)
{
  tracker_task_data_t task_data = MSG_task_get_data(data);
  tracker_task_data_free(task_data);
  MSG_task_destroy(data);
}

/**
 * Free the data structure of a tracker task.
 * @param task data to free
 */
void tracker_task_data_free(tracker_task_data_t task)
{
  xbt_dynar_free(&task->peers);
  xbt_free(task);
}

/**
 * Returns if the given id is in the peers lsit
 * @param peers dynar containing the peers
 * @param id identifier of the peer to test
 */
int is_in_list(xbt_dynar_t peers, int id)
{
  unsigned i;
  int elm;
  xbt_dynar_foreach(peers, i, elm) {
    if (elm == id) {
      return 1;
    }
  }
  return 0;
}
