/* Copyright (c) 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package io;

import org.simgrid.msg.File;
import org.simgrid.msg.Host;
import org.simgrid.msg.HostNotFoundException;
import org.simgrid.msg.Msg;
import org.simgrid.msg.MsgException;
import org.simgrid.msg.Process;

public class Node extends Process {
	private static String FILENAME1 = "/home/user/Install/simgrid/doc/simgrid/examples/platforms/g5k.xml";
	private static String FILENAME2 = "/home/user/Install/simgrid/doc/simgrid/examples/platforms/One_cluster_no_backbone.xml";
	private static String FILENAME3 = "/home/user/Install/simgrid/doc/simgrid/examples/platforms/g5k_cabinets.xml";
	private static String FILENAME4 = "/home/user/Install/simgrid/doc/simgrid/examples/platforms/nancy.xml";
			
	protected int number;
		
	public Node(Host host, int number) throws HostNotFoundException {
		super(host, Integer.toString(number), null);
		this.number = number;
	}	
	public void main(String[] args) throws MsgException {
		String mount = "c:";
		String filename;
		switch (number) {
			case 0:
				filename = mount + FILENAME1;
			break;
			case 1:
				filename = mount + FILENAME2;
			break;
			case 2:
				filename = mount + FILENAME3;
			break;
			case 3:
				filename = mount + FILENAME4;
			break;
			default:
				filename = mount + FILENAME1;
		}
		Msg.info("Open file " + filename);
		File file = new File(filename);

		long read = file.read(10000000,1);
		Msg.info("Having read " + read + " on " + filename);
		
		long write = file.read(100000,1);
		Msg.info("Having write " + write + " on " + filename);

		read = file.read(10000000,1);
		Msg.info("Having read " + read + " on " + filename);	
	}
}
