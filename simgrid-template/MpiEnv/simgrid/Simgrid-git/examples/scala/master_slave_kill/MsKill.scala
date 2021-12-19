/*
 * Copyright (c) 2006-2013. The SimGrid Team.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package.
 */

package master_slave_kill;

import org.simgrid.msg.Msg;
import org.simgrid.msg.MsgException;
import org.simgrid.msg.NativeException;

object MsKill extends App{

  /* This only contains the launcher. If you do nothing more than than you can run
  *   scala simgrid.msg.Msg
  * which also contains such a launcher
  */


  /* initialize the MSG simulation. Must be done before anything else (even logging). */
  Msg.init(args)
  Msg.createEnvironment(args(0))

  /* bypass deploymemt */
  try {
    val process1 = new Master("Jupiter","master")
    process1.start()
  }
  catch {
    case e:MsgException => println("Create processes failed!")
  }

  /*  execute the simulation. */
  Msg.run()
}
