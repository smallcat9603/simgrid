# Copyright (c) 2010-2020. The SimGrid Team. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the license (GNU LGPL) which comes with this package.

from simgrid import Comm, Engine, Mailbox, this_actor
import sys

# This example shows how to block on the completion of a set of communications.
#
# As for the other asynchronous examples, the sender initiate all the messages it wants to send and
# pack the resulting simgrid.Comm objects in a list. All messages thus occur concurrently.
#
# The sender then loops until there is no ongoing communication. Using wait_any() ensures that the sender
# will notice events as soon as they occur even if it does not follow the order of the container.
#
# Here, finalize messages will terminate earlier because their size is 0, so they travel faster than the
# other messages of this application.  As expected, the trace shows that the finalize of worker 1 is
# processed before 'Message 5' that is sent to worker 0.


class Sender:
    def __init__(self, *args):
        if len(args) != 3:
            raise AssertionError("Actor sender requires 3 parameters, but got {:d}".format(len(args)))
        self.messages_count = int(args[0])  # number of tasks
        self.msg_size = int(args[1])  # communication cost (in bytes)
        self.receivers_count = int(args[2])  # number of receivers

    def __call__(self):
        # List in which we store all ongoing communications
        pending_comms = []

        # Vector of the used mailboxes
        mboxes = [Mailbox.by_name("receiver-{:d}".format(i))
                  for i in range(0, self.receivers_count)]

        # Start dispatching all messages to receivers, in a round robin fashion
        for i in range(0, self.messages_count):
            content = "Message {:d}".format(i)
            mbox = mboxes[i % self.receivers_count]

            this_actor.info("Send '{:s}' to '{:s}'".format(content, str(mbox)))

            # Create a communication representing the ongoing communication, and store it in pending_comms
            comm = mbox.put_async(content, self.msg_size)
            pending_comms.append(comm)

        # Start sending messages to let the workers know that they should stop
        for i in range(0, self.receivers_count):
            mbox = mboxes[i]
            this_actor.info("Send 'finalize' to '{:s}'".format(str(mbox)))
            comm = mbox.put_async("finalize", 0)
            pending_comms.append(comm)

        this_actor.info("Done dispatching all messages")

        # Now that all message exchanges were initiated, wait for their completion, in order of completion.
        #
        # This loop waits for first terminating message with wait_any() and remove it with del, until all comms are
        # terminated.
        # Even in this simple example, the pending comms do not terminate in the exact same order of creation.
        while pending_comms:
            changed_pos = Comm.wait_any(pending_comms)
            del pending_comms[changed_pos]
            if (changed_pos != 0):
                this_actor.info(
                    "Remove the {:d}th pending comm: it terminated earlier than another comm that was initiated first.".format(changed_pos))

        this_actor.info("Goodbye now!")


class Receiver:
    def __init__(self, *args):
        if len(args) != 1:  # Receiver actor expects 1 argument: its ID
            raise AssertionError(
                "Actor receiver requires 1 parameter, but got {:d}".format(len(args)))
        self.mbox = Mailbox.by_name("receiver-{:s}".format(args[0]))

    def __call__(self):
        this_actor.info("Wait for my first message")
        while True:
            received = self.mbox.get()
            this_actor.info("I got a '{:s}'.".format(received))
            if received == "finalize":
                break  # If it's a finalize message, we're done.


if __name__ == '__main__':
    e = Engine(sys.argv)

    # Load the platform description
    e.load_platform(sys.argv[1])

    # Register the classes representing the actors
    e.register_actor("sender", Sender)
    e.register_actor("receiver", Receiver)

    e.load_deployment(sys.argv[2])

    e.run()
