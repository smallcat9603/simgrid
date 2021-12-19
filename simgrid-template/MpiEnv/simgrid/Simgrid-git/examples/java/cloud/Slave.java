/* Copyright (c) 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package cloud;

import org.simgrid.msg.Host;
import org.simgrid.msg.Msg;
import org.simgrid.msg.MsgException;
import org.simgrid.msg.Process;
import org.simgrid.msg.Task;

public class Slave extends Process {
	private int number;
	public Slave(Host host, int number) {
		super(host,"WRK0" + number,null);
		this.number = number;
	}
	public void main(String[] args) throws MsgException {
		Msg.info(this.msgName() +" is listenning on MBOX:WRK0"+ number);
		while(true) {  			
			Task task;
                        try {
                                task = Task.receive("MBOX:WRK0"+number);
                        } catch (MsgException e) {
                                Msg.debug("Received failed. I'm done. See you!");
                                break;
                        }
			if (task instanceof FinalizeTask) {
                                Msg.info("Received Finalize. I'm done. See you!");
				break;
			}
			Msg.info("Received \"" + task.getName() +  "\". Processing it.");
			try {
				task.execute();
			} catch (MsgException e) {

			}
			Msg.info(this.msgName() +" executed task (" + task.getName()+")");
		}

		
	}
}
