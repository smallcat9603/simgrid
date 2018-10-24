/* Copyright (c) 2017-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SMPI_HOST_HPP_
#define SMPI_HOST_HPP_

#include "smpi_utils.hpp"

#include "simgrid/s4u/Host.hpp"
#include <string>
#include <vector>
#include <xbt/Extendable.hpp>

namespace simgrid {
namespace smpi {

class Host {

  private:
  std::vector<s_smpi_factor_t> orecv_parsed_values;
  std::vector<s_smpi_factor_t> osend_parsed_values;
  std::vector<s_smpi_factor_t> oisend_parsed_values;
  simgrid::s4u::Host *host = nullptr;

  public:
  static simgrid::xbt::Extension<simgrid::s4u::Host, Host> EXTENSION_ID;

  explicit Host(simgrid::s4u::Host *ptr);
  ~Host();

  double orecv(size_t size);
  double osend(size_t size);
  double oisend(size_t size);
};

}
}
#endif
