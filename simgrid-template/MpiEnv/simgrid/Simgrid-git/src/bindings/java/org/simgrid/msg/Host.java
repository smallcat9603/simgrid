/* Bindings to the MSG hosts */

/* Copyright (c) 2006-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package org.simgrid.msg;

import org.simgrid.msg.Storage;

/**
 * A host object represents a location (any possible place) where a process may run. 
 * Thus it is represented as a physical resource with computing capabilities, some 
 * mailboxes to enable running process to communicate with remote ones, and some private 
 * data that can be only accessed by local process. An instance of this class is always 
 * bound with the corresponding native host. All the native hosts are automatically created
 * during the call of the method Msg.createEnvironment(). This method take as parameter a
 * platform file which describes all elements of the platform (host, link, root..).
 * You cannot create a host yourself.
 *
 * The best way to get an host instance is to call the static method 
 * Host.getByName().
 *
 * For example to get the instance of the host. If your platform
 * file description contains an host named "Jacquelin" :
 *
 * \verbatim
Host jacquelin;

try { 
	jacquelin = Host.getByName("Jacquelin");
} catch(HostNotFoundException e) {
	System.err.println(e.toString());
}
...
\endverbatim
 *
 */ 
public class Host {

	/**
	 * This attribute represents a bind between a java host object and
	 * a native host. Even if this attribute is public you must never
	 * access to it. It is set automatically during the call of the 
	 * static method Host.getByName().
	 *
	 * @see				Host.getByName().
	 */ 
	private long bind;
	/**
	 * Host name
	 */
	protected String name;

	/**
	 * User data.
	 */ 
	private Object data;
    /**
     *
     */
    protected Host() {
		this.bind = 0;
		this.data = null;
	};
	
	public String toString (){
		return this.name; 
		
	}

	/**
	 * This static method gets an host instance associated with a native
	 * host of your platform. This is the best way to get a java host object.
	 *
	 * @param name		The name of the host to get.
	 *
	 * @return		The host object with the given name.
     * @exception		HostNotFoundException if the name of the host is not valid.
	 *					NativeException if the native version of this method failed.
	 */ 
	public native static Host getByName(String name) 
	throws HostNotFoundException, NullPointerException;
	/**
	 * This static method returns the count of the installed hosts.
	 *
	 * @return			The count of the installed hosts.
	 */ 
	public native static int getCount();

	/**
	 * This static method return an instance to the host of the current process.
	 *
	 * @return			The host on which the current process is executed.
	 */ 
	public native static Host currentHost();

	/**
	 * This static method returns all of the hosts of the installed platform.
	 *
	 * @return			An array containing all the hosts installed.
	 *
	 */ 
	public native static Host[] all();

    /** 
     * This static method sets a mailbox to receive in asynchronous mode.
     * 
     * All messages sent to this mailbox will be transferred to 
     * the receiver without waiting for the receive call. 
     * The receive call will still be necessary to use the received data.
     * If there is a need to receive some messages asynchronously, and some not, 
     * two different mailboxes should be used.
     *
     * @param mailboxName The name of the mailbox
     */
    public static native void setAsyncMailbox(String mailboxName);


	/**
	 * This method returns the name of a host.
	 * @return			The name of the host.
	 *
	 */ 
	public String getName() {
		return name;
	}

	/**
	 * Sets the data of the host.
     * @param data
     */
	public void setData(Object data) {
		this.data = data;
	} 
	/**
	 * Gets the data of the host.
     *
     * @return The data object associated with the host.
     */
	public Object getData() {
		return this.data;
	}

	/**
	 * Checks whether a host has data.
     *
     * @return True if the host has an associated data object.
     */
	public boolean hasData() {
		return null != this.data;
	}

	/**
	 * This method start the host if it is off
	 */ 
	public native void on();

	/**
	 * This method stop the host if it is on
	 */ 
	public native void off();


	/**
	 * This method returns the number of tasks currently running on a host.
	 * The external load is not taken in account.
	 *
	 * @return			The number of tasks currently running on a host.
	 */ 
	public native int getLoad();

	/**
	 * This method returns the speed of the processor of a host,
	 * regardless of the current load of the machine.
	 *
	 * @return			The speed of the processor of the host in flops.
	 *
	 */ 
	public native double getSpeed();

	/**
	 * This method returns the number of core of a host.
	 *
	 * @return			The speed of the processor of the host in flops.
	 *
	 */ 
	public native double getCoreNumber();

	/**
	 * Returns the value of a given host property. 
	 */
	public native String getProperty(String name);
	
	/**
	 * Change the value of a given host property. 
	 */
	public native void setProperty(String name, String value);
    
	/** This method tests if a host is up and running.
	 * @return True if the host is available.
	 */
	public native boolean isOn();

	/** This methods returns the list of mount point names on an host
     * @return An array containing all mounted storages on the host
     */
	public native Storage[] getMountedStorage();

	/** This methods returns the list of storages attached to an host
     * @return An array containing all storages (name) attached to the host
     */
	public native String[] getAttachedStorage();
	
	
	/**
	 * Class initializer, to initialize various JNI stuff
	 */
	public static native void nativeInit();
	static {
		nativeInit();
	}	
} 
