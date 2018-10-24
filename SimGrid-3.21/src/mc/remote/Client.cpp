/* Copyright (c) 2015-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <cerrno>
#include <cstdlib>

#include <sys/ptrace.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <xbt/log.h>
#include <xbt/mmalloc.h>
#include <xbt/sysdep.h>

#include <simgrid/modelchecker.h>

#include "src/internal_config.h"

#include "src/mc/mc_request.hpp"
#include "src/mc/remote/Client.hpp"
#include "src/mc/remote/mc_protocol.h"

#include "src/smpi/include/private.hpp"

// We won't need those once the separation MCer/MCed is complete:
#include "src/mc/mc_smx.hpp"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(mc_client, mc, "MC client logic");

namespace simgrid {
namespace mc {

std::unique_ptr<Client> Client::instance_;

Client* Client::initialize()
{
  // We are not in MC mode:
  // TODO, handle this more gracefully.
  if (not std::getenv(MC_ENV_SOCKET_FD))
    return nullptr;

  // Do not break if we are called multiple times:
  if (instance_)
    return instance_.get();

  _sg_do_model_check = 1;

  // Fetch socket from MC_ENV_SOCKET_FD:
  char* fd_env = std::getenv(MC_ENV_SOCKET_FD);
  if (not fd_env)
    xbt_die("No MC socket passed in the environment");
  int fd =
      xbt_str_parse_int(fd_env, bprintf("Variable %s should contain a number but contains '%%s'", MC_ENV_SOCKET_FD));
  XBT_DEBUG("Model-checked application found socket FD %i", fd);

  // Check the socket type/validity:
  int type;
  socklen_t socklen = sizeof(type);
  if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &socklen) != 0)
    xbt_die("Could not check socket type");
  if (type != SOCK_SEQPACKET)
    xbt_die("Unexpected socket type %i", type);
  XBT_DEBUG("Model-checked application found expected socket type");

  instance_ = std::unique_ptr<Client>(new simgrid::mc::Client(fd));

  // Wait for the model-checker:
  errno = 0;
#if defined __linux__
  ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
#elif defined BSD
  ptrace(PT_TRACE_ME, 0, nullptr, 0);
#else
#error "no ptrace equivalent coded for this platform"
#endif
  if (errno != 0 || raise(SIGSTOP) != 0)
    xbt_die("Could not wait for the model-checker");

  instance_->handleMessages();
  return instance_.get();
}

void Client::handleDeadlockCheck(s_mc_message_t* msg)
{
  bool deadlock = false;
  if (not simix_global->process_list.empty()) {
    deadlock = true;
    for (auto const& kv : simix_global->process_list)
      if (simgrid::mc::actor_is_enabled(kv.second)) {
        deadlock = false;
        break;
      }
  }

  // Send result:
  s_mc_message_int_t answer{MC_MESSAGE_DEADLOCK_CHECK_REPLY, deadlock};
  xbt_assert(channel_.send(answer) == 0, "Could not send response");
}
void Client::handleContinue(s_mc_message_t* msg)
{
  /* Nothing to do */
}
void Client::handleSimcall(s_mc_message_simcall_handle_t* message)
{
  smx_actor_t process = SIMIX_process_from_PID(message->pid);
  if (not process)
    xbt_die("Invalid pid %lu", (unsigned long)message->pid);
  SIMIX_simcall_handle(&process->simcall, message->value);
  if (channel_.send(MC_MESSAGE_WAITING))
    xbt_die("Could not send MESSAGE_WAITING to model-checker");
}
void Client::handleRestore(s_mc_message_restore_t* message)
{
#if HAVE_SMPI
  smpi_really_switch_data_segment(simgrid::s4u::Actor::by_pid(message->index));
#endif
}
void Client::handleActorEnabled(s_mc_message_actor_enabled_t* msg)
{
  bool res = simgrid::mc::actor_is_enabled(SIMIX_process_from_PID(msg->aid));
  s_mc_message_int_t answer{MC_MESSAGE_ACTOR_ENABLED_REPLY, res};
  channel_.send(answer);
}

void Client::handleMessages()
{
  while (1) {
    XBT_DEBUG("Waiting messages from model-checker");

    char message_buffer[MC_MESSAGE_LENGTH];
    ssize_t received_size = channel_.receive(&message_buffer, sizeof(message_buffer));

    if (received_size < 0)
      xbt_die("Could not receive commands from the model-checker");

    s_mc_message_t* message = (s_mc_message_t*)message_buffer;
    switch (message->type) {

      case MC_MESSAGE_DEADLOCK_CHECK:
        xbt_assert(received_size == sizeof(s_mc_message_t), "Unexpected size for DEADLOCK_CHECK (%zd != %zu)",
                   received_size, sizeof(s_mc_message_t));
        handleDeadlockCheck(message);
        break;

      case MC_MESSAGE_CONTINUE:
        xbt_assert(received_size == sizeof(s_mc_message_t), "Unexpected size for MESSAGE_CONTINUE (%zd != %zu)",
                   received_size, sizeof(s_mc_message_t));
        handleContinue(message);
        return;

      case MC_MESSAGE_SIMCALL_HANDLE:
        xbt_assert(received_size == sizeof(s_mc_message_simcall_handle_t),
                   "Unexpected size for SIMCALL_HANDLE (%zd != %zu)", received_size,
                   sizeof(s_mc_message_simcall_handle_t));
        handleSimcall((s_mc_message_simcall_handle_t*)message_buffer);
        break;

      case MC_MESSAGE_RESTORE:
        xbt_assert(received_size == sizeof(s_mc_message_t), "Unexpected size for MESSAGE_RESTORE (%zd != %zu)",
                   received_size, sizeof(s_mc_message_t));
        handleRestore((s_mc_message_restore_t*)message_buffer);
        break;

      case MC_MESSAGE_ACTOR_ENABLED:
        xbt_assert(received_size == sizeof(s_mc_message_actor_enabled_t),
                   "Unexpected size for ACTOR_ENABLED (%zd != %zu)", received_size,
                   sizeof(s_mc_message_actor_enabled_t));
        handleActorEnabled((s_mc_message_actor_enabled_t*)message_buffer);
        break;

      default:
        xbt_die("Received unexpected message %s (%i)", MC_message_type_name(message->type), message->type);
        break;
    }
  }
}

void Client::mainLoop()
{
  while (1) {
    simgrid::mc::wait_for_requests();
    xbt_assert(channel_.send(MC_MESSAGE_WAITING) == 0, "Could not send WAITING message to model-checker");
    this->handleMessages();
  }
}

void Client::reportAssertionFailure(const char* description)
{
  if (channel_.send(MC_MESSAGE_ASSERTION_FAILED))
    xbt_die("Could not send assertion to model-checker");
  this->handleMessages();
}

void Client::ignoreMemory(void* addr, std::size_t size)
{
  s_mc_message_ignore_memory_t message;
  message.type = MC_MESSAGE_IGNORE_MEMORY;
  message.addr = (std::uintptr_t)addr;
  message.size = size;
  if (channel_.send(message))
    xbt_die("Could not send IGNORE_MEMORY mesage to model-checker");
}

void Client::ignoreHeap(void* address, std::size_t size)
{
  xbt_mheap_t heap = mmalloc_get_current_heap();

  s_mc_message_ignore_heap_t message;
  message.type    = MC_MESSAGE_IGNORE_HEAP;
  message.address = address;
  message.size    = size;
  message.block   = ((char*)address - (char*)heap->heapbase) / BLOCKSIZE + 1;
  if (heap->heapinfo[message.block].type == 0) {
    message.fragment = -1;
    heap->heapinfo[message.block].busy_block.ignore++;
  } else {
    message.fragment = ((uintptr_t)(ADDR2UINT(address) % (BLOCKSIZE))) >> heap->heapinfo[message.block].type;
    heap->heapinfo[message.block].busy_frag.ignore[message.fragment]++;
  }

  if (channel_.send(message))
    xbt_die("Could not send ignored region to MCer");
}

void Client::unignoreHeap(void* address, std::size_t size)
{
  s_mc_message_ignore_memory_t message;
  message.type = MC_MESSAGE_UNIGNORE_HEAP;
  message.addr = (std::uintptr_t)address;
  message.size = size;
  if (channel_.send(message))
    xbt_die("Could not send IGNORE_HEAP message to model-checker");
}

void Client::declareSymbol(const char* name, int* value)
{
  s_mc_message_register_symbol_t message;
  message.type = MC_MESSAGE_REGISTER_SYMBOL;
  if (strlen(name) + 1 > sizeof(message.name))
    xbt_die("Symbol is too long");
  strncpy(message.name, name, sizeof(message.name));
  message.callback = nullptr;
  message.data     = value;
  if (channel_.send(message))
    xbt_die("Could send REGISTER_SYMBOL message to model-checker");
}

void Client::declareStack(void* stack, size_t size, smx_actor_t process, ucontext_t* context)
{
  xbt_mheap_t heap = mmalloc_get_current_heap();

  s_stack_region_t region;
  memset(&region, 0, sizeof(region));
  region.address = stack;
  region.context = context;
  region.size    = size;
  region.block   = ((char*)stack - (char*)heap->heapbase) / BLOCKSIZE + 1;
#if HAVE_SMPI
  if (smpi_privatize_global_variables == SmpiPrivStrategies::MMAP && process)
    region.process_index = process->pid_ - 1;
  else
#endif
    region.process_index = -1;

  s_mc_message_stack_region_t message;
  message.type         = MC_MESSAGE_STACK_REGION;
  message.stack_region = region;
  if (channel_.send(message))
    xbt_die("Could not send STACK_REGION to model-checker");
}
}
}
