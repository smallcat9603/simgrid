/* Copyright (c) 2006-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package tracing;
import org.simgrid.msg.NativeException;
import org.simgrid.msg.Task;

public class PingPongTask extends Task {
   
   private double timeVal;
   
   public PingPongTask() throws NativeException {
      this.timeVal = 0;
   }
   
   public PingPongTask(String name, double computeDuration, double messageSize) throws NativeException {      
      super(name,computeDuration,messageSize);		
   }
   
   public void setTime(double timeVal){
      this.timeVal = timeVal;
   }
   
   public double getTime() {
      return this.timeVal;
   }
}
    
