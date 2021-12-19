/*
 * Master of a basic master/slave example in Java
 *
 * Copyright (c) 2006-2013. The SimGrid Team.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package.
 */

package master_slave_bypass

import Stream._
import org.simgrid.msg.HostNotFoundException
import org.simgrid.msg.Msg
import org.simgrid.msg.MsgException
import org.simgrid.msg.Process
import org.simgrid.msg.Task

class Master(hostname:String, name:String) extends Process(hostname,name) {

  def main(args:Array[String]) {
    Msg.info("Master Hello!")

    //Create a slave on host "alice"
    try {
      Msg.info("Create process on host 'Tremblay'")
      new Slave("Tremblay","process2").start()
    } catch {
      case e:MsgException => println("Process2!")
    }

    //Wait for slave "alice"
    continually({Task.receive("Tremblay")})
      .takeWhile(!_.isInstanceOf[FinalizeTask])
      .force // to force the list to be compute

    Msg.info("Received Finalize. I'm done. See you!")
  }
}
