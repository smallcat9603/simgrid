/* Copyright (c) 2006-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package async;
import org.simgrid.msg.Host;
import org.simgrid.msg.Msg;
import org.simgrid.msg.MsgException;
import org.simgrid.msg.Task;
import org.simgrid.msg.Process;


public class Forwarder extends Process {
	public Forwarder(Host host, String name, String[]args) {
		super(host,name,args);
	}
	public void main(String[] args) throws MsgException {
		if (args.length < 3) {	 
			Msg.info("Forwarder needs 3 arguments (input mailbox, first output mailbox, last one)");
			Msg.info("Got "+args.length+" instead");
			System.exit(1);
		}
		int input = Integer.valueOf(args[0]).intValue();		
		int firstOutput = Integer.valueOf(args[1]).intValue();		
		int lastOutput = Integer.valueOf(args[2]).intValue();		

		int taskCount = 0;
		int slavesCount = lastOutput - firstOutput + 1;
		Msg.info("Receiving on 'slave_"+input+"'");
		while(true) {
			Task task = Task.receive("slave_"+input);	

			if (task instanceof FinalizeTask) {
				Msg.info("Got a finalize task. Let's forward (asynchronously) that we're done, and then sleep 20 seconds so that nobody gets a message from a terminated process.");

				for (int cpt = firstOutput; cpt<=lastOutput; cpt++) {
					Task tf = new FinalizeTask();
					tf.dsend("slave_"+cpt);
				}
				waitFor(20);
				break;
			}
			int dest = firstOutput + (taskCount % slavesCount);

			Msg.info("Sending \"" + task.getName() + "\" to \"slave_" + dest + "\"");
			task.send("slave_"+dest);

			taskCount++;
		}


		Msg.info("I'm done. See you!");
	}
}

