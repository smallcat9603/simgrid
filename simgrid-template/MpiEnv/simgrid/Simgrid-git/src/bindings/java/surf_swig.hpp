/* Copyright (c) 2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <cstdio>
#include <iostream>
#include "src/surf/cpu_interface.hpp"
#include "src/surf/network_interface.hpp"
#include "src/surf/maxmin_private.hpp"

typedef xbt_dynar_t NetworkLinkDynar;
typedef ActionList *ActionArrayPtr;

double getClock();

void clean();

CpuModel *getCpuModel();
void setCpuModel(CpuModel *cpuModel);

void setCpu(char *name, Cpu *cpu);

NetworkLinkDynar getRoute(char *srcName, char *dstName);

class Plugin {
public:
 virtual ~Plugin() {
   std::cout << "Plugin::~Plugin()" << std:: endl;
 }

 void activateCpuCreatedCallback();
 virtual void cpuCreatedCallback(Cpu *cpu) {}

 void activateCpuDestructedCallback();
 virtual void cpuDestructedCallback(Cpu *cpu) {}

 void activateCpuStateChangedCallback();
 virtual void cpuStateChangedCallback(Cpu *cpu, e_surf_resource_state_t, e_surf_resource_state_t) {}

 void activateCpuActionStateChangedCallback();
 virtual void cpuActionStateChangedCallback(CpuAction *action, e_surf_action_state_t, e_surf_action_state_t) {}


 void activateNetworkLinkCreatedCallback();
 virtual void networkLinkCreatedCallback(NetworkLink *link) {}

 void activateNetworkLinkDestructedCallback();
 virtual void networkLinkDestructedCallback(NetworkLink *link) {}

 void activateNetworkLinkStateChangedCallback();
 virtual void networkLinkStateChangedCallback(NetworkLink *link, e_surf_resource_state_t, e_surf_resource_state_t) {}

 void activateNetworkActionStateChangedCallback();
 virtual void networkActionStateChangedCallback(NetworkAction *action, e_surf_action_state_t old, e_surf_action_state_t cur) {}

 void activateNetworkCommunicateCallback();
 virtual void networkCommunicateCallback(NetworkAction *action, RoutingEdge *src, RoutingEdge *dst, double size, double rate) {}
};
