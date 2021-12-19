/* JNI interface to C code for MSG. */

/* Copyright (c) 2006-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package org.simgrid.msg;
import org.simgrid.NativeLib;

import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.File;


public final class Msg {
	/* Statically load the library which contains all native functions used in here */
	static private boolean isNativeInited = false;
        public static void nativeInit() {
                if (isNativeInited)
                      return;
                NativeLib.nativeInit("simgrid");
                NativeLib.nativeInit("simgrid-java");      
		isNativeInited = true;

	}

	static {
                nativeInit();
	}
        
    /** Retrieve the simulation time
     * @return The simulation time.
     */
	public final static native double getClock();
	/**
	 * Issue a debug logging message.
	 * @param s message to log.
	 */
	public final static native void debug(String s);
	/**
	 * Issue an verbose logging message.
	 * @param s message to log.
	 */
	public final static native void verb(String s);

	/** Issue an information logging message
     * @param s
     */
	public final static native void info(String s);
	/**
	 * Issue an warning logging message.
	 * @param s message to log.
	 */
	public final static native void warn(String s);
	/**
	 * Issue an error logging message.
	 * @param s message to log.
	 */
	public final static native void error(String s);
	/**
	 * Issue an critical logging message.
	 * @param s message to log.
	 */
	public final static native void critical(String s);

	/*********************************************************************************
	 * Deployment and initialization related functions                               *
	 *********************************************************************************/

	/**
	 * The natively implemented method to initialize a MSG simulation.
	 *
	 * @param args            The arguments of the command line of the simulation.
	 */
	public final static native void init(String[]args);

	/**
	 * Run the MSG simulation.
	 *
	 * The simulation is not cleaned afterward (see  
	 * {@link #clean()} if you really insist on cleaning the C side), so you can freely 
	 * retrieve the informations that you want from the simulation. In particular, retrieving the status 
	 * of a process or the current date is perfectly ok. 
	 */
	public final static native void run() ;
	
	/** This function is useless nowadays, just stop calling it. */
	@Deprecated
	public final static void clean(){}

	/**
	 * The native implemented method to create the environment of the simulation.
	 *
	 * @param platformFile    The XML file which contains the description of the environment of the simulation
	 *
	 */
	public final static native void createEnvironment(String platformFile);

	public final static native As environmentGetRoutingRoot();

	/**
	 * The method to deploy the simulation.
	 *
     *
     * @param deploymentFile
     */
	public final static native void deployApplication(String deploymentFile);

    /** Example launcher. You can use it or provide your own launcher, as you wish
     * @param args
     * @throws MsgException
     */
	static public void main(String[]args) throws MsgException {
		/* initialize the MSG simulation. Must be done before anything else (even logging). */
		Msg.init(args);

		if (args.length < 2) {
			Msg.info("Usage: Msg platform_file deployment_file");
			System.exit(1);
		}

		/* Load the platform and deploy the application */
		Msg.createEnvironment(args[0]);
		Msg.deployApplication(args[1]);
		/* Execute the simulation */
		Msg.run();
	}
}
