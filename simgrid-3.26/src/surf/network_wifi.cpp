/* Copyright (c) 2019-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "network_wifi.hpp"
#include "simgrid/s4u/Host.hpp"
#include "src/surf/surf_interface.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(surf_network);

namespace simgrid {
namespace kernel {
namespace resource {

/************
 * Resource *
 ************/

NetworkWifiLink::NetworkWifiLink(NetworkCm02Model* model, const std::string& name, std::vector<double> bandwidths,
                                 lmm::System* system)
    : LinkImpl(model, name, system->constraint_new(this, 1))
{
  for (auto bandwidth : bandwidths)
    bandwidths_.push_back({bandwidth, 1.0, nullptr});

  simgrid::s4u::Link::on_creation(*get_iface());
}

void NetworkWifiLink::set_host_rate(const s4u::Host* host, int rate_level)
{
  auto insert_done = host_rates_.insert(std::make_pair(host->get_name(), rate_level));
  if (not insert_done.second)
    insert_done.first->second = rate_level;

  // Each time we add a host, we refresh the decay model
  refresh_decay_bandwidths();
}

double NetworkWifiLink::get_host_rate(const s4u::Host* host) const
{
  auto host_rates_it = host_rates_.find(host->get_name());

  if (host_rates_it == host_rates_.end())
    return -1;

  int rate_id = host_rates_it->second;
  xbt_assert(rate_id >= 0,
             "Negative host wifi rate levels are invalid but host '%s' uses %d as a rate level on link '%s'",
             host->get_cname(), rate_id, this->get_cname());
  xbt_assert(rate_id < (int)bandwidths_.size(),
             "Link '%s' only has %zu wifi rate levels, so the provided level %d is invalid for host '%s'.",
             this->get_cname(), bandwidths_.size(), rate_id, host->get_cname());

  Metric rate = use_decay_model_ ? decay_bandwidths_[rate_id] : bandwidths_[rate_id];
  return rate.peak * rate.scale;
}

s4u::Link::SharingPolicy NetworkWifiLink::get_sharing_policy() const
{
  return s4u::Link::SharingPolicy::WIFI;
}

int NetworkWifiLink::get_host_count() const
{
  return static_cast<int>(host_rates_.size());
}

void NetworkWifiLink::refresh_decay_bandwidths(){
  // Compute number of STAtion on the Access Point
  int nSTA = get_host_count();

  std::vector<Metric> new_bandwidths;
  for (auto bandwidth : bandwidths_){
    // Instantiate decay model relatively to the actual bandwidth
    double max_bw=bandwidth.peak;
    double min_bw=bandwidth.peak-(wifi_max_rate_-wifi_min_rate_);
    double model_rate=bandwidth.peak-(wifi_max_rate_-model_rate_);

    xbt_assert(min_bw > 0, "Your WIFI link is using bandwidth(s) which is too low for the decay model.");

    double N0=max_bw-min_bw;
    double lambda=(-log(model_rate-min_bw)+log(N0))/model_n_;
    // Since decay model start at 0 we should use (nSTA-1)
    double new_peak=N0*exp(-lambda*(nSTA-1))+min_bw;
    new_bandwidths.push_back({new_peak, 1.0, nullptr});
  }
  decay_bandwidths_=new_bandwidths;
}

bool NetworkWifiLink::toggle_decay_model(){
  use_decay_model_=!use_decay_model_;
  return(use_decay_model_);
}

  
} // namespace resource
} // namespace kernel
} // namespace simgrid
