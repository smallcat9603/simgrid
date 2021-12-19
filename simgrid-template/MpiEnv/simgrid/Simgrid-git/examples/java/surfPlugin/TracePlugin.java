/* Copyright (c) 2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package surfPlugin;

import org.simgrid.surf.*;
import org.simgrid.msg.Msg;
import java.util.HashMap;

public class TracePlugin extends Plugin {

  public TracePlugin() {
    activateCpuCreatedCallback();
    activateCpuDestructedCallback();
    activateCpuStateChangedCallback();
    activateCpuActionStateChangedCallback();

    activateNetworkLinkCreatedCallback();
    activateNetworkLinkDestructedCallback();
    activateNetworkLinkStateChangedCallback();
    activateNetworkActionStateChangedCallback();
  }

  public void cpuCreatedCallback(Cpu cpu) {
    Msg.info("Trace: Cpu created "+cpu.getName());
  }

  public void cpuDestructedCallback(Cpu cpu) {
    Msg.info("Trace: Cpu destructed "+cpu.getName());
  }

  public void cpuStateChangedCallback(Cpu cpu, ResourceState old, ResourceState cur){
    Msg.info("Trace: Cpu state changed "+cpu.getName());
  }

  public void cpuActionStateChangedCallback(CpuAction action, ActionState old, ActionState cur){
    Msg.info("Trace: CpuAction state changed "+action.getModel().getName());
  }

  public void networkLinkCreatedCallback(NetworkLink link) {
    Msg.info("Trace: NetworkLink created "+link.getName());
  }

  public void networkLinkDestructedCallback(NetworkLink link) {
    Msg.info("Trace: NetworkLink destructed "+link.getName());
  }

  public void networkLinkStateChangedCallback(NetworkLink link, ResourceState old, ResourceState cur){
    Msg.info("Trace: NetworkLink state changed "+link.getName());
  }

  public void networkActionStateChangedCallback(NetworkAction action, ActionState old, ActionState cur){
    Msg.info("Trace: NetworkAction state changed "+action.getModel().getName());
  }

}
