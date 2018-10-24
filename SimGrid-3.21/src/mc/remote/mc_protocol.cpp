/* Copyright (c) 2015-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <cerrno>
#include <cstddef> // std::size_t
#include <cstdio>  // perror
#include <cstring>

#include <sys/socket.h>
#include <sys/types.h>

#include <xbt/log.h>

#include "src/mc/remote/Client.hpp"
#include "src/mc/remote/mc_protocol.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(mc_protocol, mc, "Generic MC protocol logic");

const char* MC_message_type_name(e_mc_message_type type)
{
  switch (type) {
    case MC_MESSAGE_NONE:
      return "NONE";
    case MC_MESSAGE_CONTINUE:
      return "CONTINUE";
    case MC_MESSAGE_IGNORE_HEAP:
      return "IGNORE_HEAP";
    case MC_MESSAGE_UNIGNORE_HEAP:
      return "UNIGNORE_HEAP";
    case MC_MESSAGE_IGNORE_MEMORY:
      return "IGNORE_MEMORY";
    case MC_MESSAGE_STACK_REGION:
      return "STACK_REGION";
    case MC_MESSAGE_REGISTER_SYMBOL:
      return "REGISTER_SYMBOL";
    case MC_MESSAGE_DEADLOCK_CHECK:
      return "DEADLOCK_CHECK";
    case MC_MESSAGE_DEADLOCK_CHECK_REPLY:
      return "DEADLOCK_CHECK_REPLY";
    case MC_MESSAGE_WAITING:
      return "WAITING";
    case MC_MESSAGE_SIMCALL_HANDLE:
      return "SIMCALL_HANDLE";
    case MC_MESSAGE_ASSERTION_FAILED:
      return "ASSERTION_FAILED";

    case MC_MESSAGE_ACTOR_ENABLED:
      return "ACTOR_ENABLED";
    case MC_MESSAGE_ACTOR_ENABLED_REPLY:
      return "ACTOR_ENABLED_REPLY";

    default:
      return "?";
  }
}
