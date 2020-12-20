/* Copyright (c) 2010-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/actor.h"
#include "simgrid/comm.h"
#include "simgrid/engine.h"
#include "simgrid/host.h"
#include "simgrid/mailbox.h"

#include "xbt/asserts.h"
#include "xbt/log.h"
#include "xbt/str.h"

#include <stdio.h> /* snprintf */

XBT_LOG_NEW_DEFAULT_CATEGORY(comm_waitall, "Messages specific for this msg example");

static void sender(int argc, char* argv[])
{
  xbt_assert(argc == 4, "This function expects 3 parameters from the XML deployment file");
  long messages_count  = xbt_str_parse_int(argv[1], "Invalid message count: %s");
  long message_size    = xbt_str_parse_int(argv[2], "Invalid message size: %s");
  long receivers_count = xbt_str_parse_int(argv[3], "Invalid amount of receivers: %s");
  xbt_assert(receivers_count > 0);

  /* Array in which we store all ongoing communications */
  sg_comm_t* pending_comms = xbt_malloc(sizeof(sg_comm_t) * (messages_count + receivers_count));
  int pending_comms_count  = 0;

  /* Make an array of the mailboxes to use */
  sg_mailbox_t* mboxes = xbt_malloc(sizeof(sg_mailbox_t) * receivers_count);
  for (long i = 0; i < receivers_count; i++) {
    char mailbox_name[80];
    snprintf(mailbox_name, 79, "receiver-%ld", i);
    sg_mailbox_t mbox = sg_mailbox_by_name(mailbox_name);
    mboxes[i]         = mbox;
  }

  /* Start dispatching all messages to receivers, in a round robin fashion */
  for (long i = 0; i < messages_count; i++) {
    char msg_content[80];
    snprintf(msg_content, 79, "Message %ld", i);
    sg_mailbox_t mbox = mboxes[i % receivers_count];
    XBT_INFO("Send '%s' to '%s'", msg_content, sg_mailbox_get_name(mbox));
    /* Create a communication representing the ongoing communication, and store it in pending_comms */
    pending_comms[pending_comms_count++] = sg_mailbox_put_async(mbox, xbt_strdup(msg_content), message_size);
  }

  /* Start sending messages to let the workers know that they should stop */
  for (long i = 0; i < receivers_count; i++) {
    XBT_INFO("Send 'finalize' to 'receiver-%ld'", i);
    char* end_msg                        = xbt_strdup("finalize");
    sg_mailbox_t mbox                    = mboxes[i % receivers_count];
    pending_comms[pending_comms_count++] = sg_mailbox_put_async(mbox, end_msg, 0);
  }

  XBT_INFO("Done dispatching all messages");

  /* Now that all message exchanges were initiated, wait for their completion in one single call */
  sg_comm_wait_all(pending_comms, pending_comms_count);

  free(pending_comms);
  free(mboxes);

  XBT_INFO("Goodbye now!");
}

static void receiver(int argc, char* argv[])
{
  xbt_assert(argc == 2, "Expecting one parameter from the XML deployment file but got %d", argc);
  int id = (int)xbt_str_parse_int(argv[1], "ID should be numerical, not %s");
  char mailbox_name[80];
  snprintf(mailbox_name, 79, "receiver-%d", id);
  sg_mailbox_t mbox = sg_mailbox_by_name(mailbox_name);
  XBT_INFO("Wait for my first message");
  while (1) {
    char* received = (char*)sg_mailbox_get(mbox);
    XBT_INFO("I got a '%s'.", received);
    if (!strcmp(received, "finalize")) { // If it's a finalize message, we're done
      xbt_free(received);
      break;
    }
    xbt_free(received);
  }
}

int main(int argc, char* argv[])
{
  simgrid_init(&argc, argv);
  xbt_assert(argc > 2,
             "Usage: %s platform_file deployment_file\n"
             "\tExample: %s msg_platform.xml msg_deployment.xml\n",
             argv[0], argv[0]);

  simgrid_load_platform(argv[1]);

  simgrid_register_function("sender", sender);
  simgrid_register_function("receiver", receiver);
  simgrid_load_deployment(argv[2]);

  simgrid_run();

  return 0;
}
