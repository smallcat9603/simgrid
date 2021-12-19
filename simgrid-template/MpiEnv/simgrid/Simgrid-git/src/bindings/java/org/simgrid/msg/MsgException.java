/* This exception is an abstract class grouping all MSG-related exceptions */

/* Copyright (c) 2006-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package org.simgrid.msg;

/**
 * This exception is an abstract class grouping all MSG-related exceptions
 *
 *  <!--
 *    DOXYGEN_NAVBAR_CHILD "HostNotFoundException"=classsimgrid_1_1msg_1_1HostNotFoundException.html
 *    DOXYGEN_NAVBAR_CHILD "JniException"=classsimgrid_1_1msg_1_1JniException.html
 *    DOXYGEN_NAVBAR_CHILD "NativeException"=classsimgrid_1_1msg_1_1NativeException.html
 *    DOXYGEN_NAVBAR_CHILD "ProcessNotFoundException"=classsimgrid_1_1msg_1_1ProcessNotFoundException.html
 *  -->
 */
public abstract class MsgException extends Exception {
  private static final long serialVersionUID = 1L;

  /**
   * Constructs an <code>MsgException</code> without a 
   * detail message. 
   */
  public MsgException() {
    super();
  }
  /**
   * Constructs an <code>MsgException</code> with a detail message. 
   *
   * @param   s   the detail message.
   */ 
  public MsgException(String s) {
    super(s);
  }
}
