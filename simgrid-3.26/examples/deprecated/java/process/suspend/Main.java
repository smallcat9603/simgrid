/* Copyright (c) 2006-2020. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package process.suspend;
import org.simgrid.msg.Msg;
import org.simgrid.msg.MsgException;

public class Main {
  private Main() {
    throw new IllegalAccessError("Utility class");
  }

  public static void main(String[] args) {
    Msg.init(args);
    if(args.length < 1) {
      Msg.info("Usage   : Suspend platform_file");
      Msg.info("example : Suspend ../platforms/platform.xml");
      System.exit(1);
    }
    /* construct the platform and deploy the application */
    Msg.createEnvironment(args[0]);
    try {
        DreamMaster process1 = new DreamMaster("Jacquelin","DreamMaster");
        process1.start();
    } catch (MsgException e){
      Msg.error("Create processes failed!");
    }

    /*  execute the simulation. */
    Msg.run();
  }
}
