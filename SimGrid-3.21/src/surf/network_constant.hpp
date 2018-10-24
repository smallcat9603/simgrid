/* Copyright (c) 2013-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef NETWORK_CONSTANT_HPP_
#define NETWORK_CONSTANT_HPP_

#include <xbt/base.h>

#include "network_interface.hpp"

namespace simgrid {
namespace kernel {
namespace resource {

class NetworkConstantModel : public NetworkModel {
public:
  NetworkConstantModel();
  Action* communicate(simgrid::s4u::Host* src, simgrid::s4u::Host* dst, double size, double rate) override;
  double next_occuring_event(double now) override;
  void update_actions_state(double now, double delta) override;

  LinkImpl* create_link(const std::string& name, double bw, double lat, s4u::Link::SharingPolicy policy) override;
};

class NetworkConstantAction : public NetworkAction {
public:
  NetworkConstantAction(NetworkConstantModel* model_, double size, double latency);
  ~NetworkConstantAction();
  double initial_latency_;
  void update_remains_lazy(double now) override;
};

} // namespace resource
} // namespace kernel
} // namespace simgrid

#endif /* NETWORK_CONSTANT_HPP_ */
