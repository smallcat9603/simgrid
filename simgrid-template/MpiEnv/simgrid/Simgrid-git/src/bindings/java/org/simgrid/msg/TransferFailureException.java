/* This exception is raised when looking for a non-existing host. */

/* Copyright (c) 2006-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package org.simgrid.msg;

/**
 * This exception is raised if transfer failed while sending tasks.
 */
public class TransferFailureException extends MsgException {
	private static final long serialVersionUID = 1L;

	/** Constructs an <code>TransferFailureException</code> without a detail message. */ 
	public TransferFailureException() {
		super();
	}
	/**
	 * Constructs an <code>TransferFailureException</code> with a detail message. 
	 *
	 * @param   s   the detail message.
	 */
	public TransferFailureException(String s) {
		super(s);
	}
}
