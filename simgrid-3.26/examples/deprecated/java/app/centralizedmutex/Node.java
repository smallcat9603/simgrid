/* Copyright (c) 2012-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package app.centralizedmutex;

import org.simgrid.msg.Msg;
import org.simgrid.msg.Host;
import org.simgrid.msg.Task;
import org.simgrid.msg.Process;
import org.simgrid.msg.MsgException;

public class Node extends Process {
  public Node(Host host, String name, String[]args) {
   super(host,name,args);
  }
  public void request(double csTime) throws MsgException {
    RequestTask req = new RequestTask(getName());
    Msg.info("Send a request to the coordinator");
    req.send("coordinator");
    Msg.info("Wait for a grant from the coordinator");
    Task.receive(getName());
    Task compute = new Task("CS", csTime, 0);
    compute.execute();
    ReleaseTask release = new ReleaseTask();
    release.send("coordinator");
  }

  public void main(String[] args) throws MsgException {
    request(Double.parseDouble(args[1]));
  }
}