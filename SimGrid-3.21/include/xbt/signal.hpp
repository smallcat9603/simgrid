/* Copyright (c) 2014-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_XBT_SIGNAL_HPP
#define SIMGRID_XBT_SIGNAL_HPP

#include <functional>
#include <utility>
#include <vector>

namespace simgrid {
namespace xbt {

  template<class S> class signal;

  /** A signal/slot mechanism
  *
  *  S is expected to be the function signature of the signal.
  *  I'm not sure we need a return value (it is currently ignored).
  *  If we don't we might use `signal<P1, P2, ...>` instead.
  */
  template<class R, class... P>
  class signal<R(P...)> {
  private:
    typedef std::function<R(P...)> callback_type;
    std::vector<callback_type> handlers_;
  public:
    template<class U>
    void connect(U slot)
    {
      handlers_.push_back(std::move(slot));
    }
    R operator()(P... args) const
    {
      for (auto const& handler : handlers_)
        handler(args...);
    }
    void disconnect_slots() { handlers_.clear(); }
    int get_slot_count() { return handlers_.size(); }
    // deprecated
    XBT_ATTRIB_DEPRECATED_v323("Please use xbt::disconnect_slots)") void disconnectSlots() { disconnect_slots(); }
    XBT_ATTRIB_DEPRECATED_v323("Please use xbt::get_slot_count)") int getSlotsAmount() { return get_slot_count(); }
  };

}
}

#endif
