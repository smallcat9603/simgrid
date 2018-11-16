/* Copyright (c) 2012-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package app.bittorrent;

import org.simgrid.msg.Msg;
import org.simgrid.msg.RngStream;

class Main{
  private Main() {
    throw new IllegalAccessError("Utility class");
  }

  public static void main(String[] args) {
    int[] seed = { 12345, 12345, 12345, 12345, 12345, 12345 };
    RngStream.setPackageSeed(seed);

    Msg.init(args);
    if(args.length < 2) {
      Msg.info("Usage   : Bittorrent platform_file deployment_file");
      Msg.info("example : Bittorrent ../platforms/cluster_backbone.xml bittorrent.xml");
      System.exit(1);
    }

    /* construct the platform and deploy the application */
    Msg.createEnvironment(args[0]);
    Msg.deployApplication(args[1]);

    /*  execute the simulation. */
    Msg.run();
  }
}
