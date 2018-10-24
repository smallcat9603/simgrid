/* xbt/replay_reader.h -- Tools to parse a replay file                */

/* Copyright (c) 2010-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef XBT_REPLAY_HPP
#define XBT_REPLAY_HPP

#include <xbt/misc.h> /* SG_BEGIN_DECL */

#include <fstream>
#include <functional>
#include <queue>
#include <unordered_map>

namespace simgrid {
namespace xbt {
/* To split the file if a unique one is given (specific variable for the other case live in runner()) */
typedef std::vector<std::string> ReplayAction;

XBT_PUBLIC_DATA std::ifstream* action_fs;
XBT_PUBLIC int replay_runner(int argc, char* argv[]);
}
}

typedef std::function<void(simgrid::xbt::ReplayAction&)> action_fun;
XBT_PUBLIC void xbt_replay_action_register(const char* action_name, action_fun function);
XBT_PUBLIC action_fun xbt_replay_action_get(const char* action_name);

#endif
