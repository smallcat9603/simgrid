/*
 * Copyright (c) 2006-2013. The SimGrid Team.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. 
 */

package masterslave

import org.simgrid.msg.Msg
import org.simgrid.msg.NativeException
import org.simgrid.msg.Task

object Masterslave extends App {
   val TASK_COMP_SIZE = 10000000
   val TASK_COMM_SIZE = 10000000
   /* This only contains the launcher. If you do nothing more than than you can run 
    *   scala simgrid.msg.Msg
    * which also contains such a launcher
    */

     /* initialize the MSG simulation. Must be done before anything else (even logging). */
     Msg.init(args)
	
     if (args.length < 2) {    		
	Msg.info("Usage   : Masterslave platform_file deployment_file")
	Msg.info("example : Masterslave basic_platform.xml basic_deployment.xml")
	System.exit(1)
     }
		
     /* construct the platform and deploy the application */
     Msg.createEnvironment(args(0))
     Msg.deployApplication(args(1))
     /*  execute the simulation. */
     Msg.run()
}
