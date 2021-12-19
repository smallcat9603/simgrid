/* Copyright (c) 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package org.simgrid.msg;
/** A mutex  implemented on top of SimGrid synchronization mechanisms. 
 * You can use it exactly the same way that you use the mutexes, 
 * but to handle the interactions between the processes within the simulation.   
 *
 */
public class Mutex {
	private long bind; // The C object -- don't touch it
	
	public Mutex() {
		init();
	}
	protected void finalize() {
		exit();
	}
	private native void exit();
	private native void init();
	public native void acquire();
	public native void release();
	
	/**
	 * Class initializer, to initialize various JNI stuff
	 */
	public static native void nativeInit();
	static {
		Msg.nativeInit();
		nativeInit();
	}	
}


