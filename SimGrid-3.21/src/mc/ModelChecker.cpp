/* Copyright (c) 2008-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <cassert>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ptrace.h>

#include <memory>
#include <system_error>

#include "xbt/automaton.h"
#include "xbt/automaton.hpp"
#include "xbt/log.h"
#include "xbt/system_error.hpp"

#include "simgrid/sg_config.hpp"

#include "src/mc/ModelChecker.hpp"
#include "src/mc/Transition.hpp"
#include "src/mc/checker/Checker.hpp"
#include "src/mc/mc_exit.hpp"
#include "src/mc/mc_private.hpp"
#include "src/mc/mc_record.hpp"
#include "src/mc/remote/RemoteClient.hpp"
#include "src/mc/remote/mc_protocol.h"
#include "src/mc/sosp/PageStore.hpp"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(mc_ModelChecker, mc, "ModelChecker");

::simgrid::mc::ModelChecker* mc_model_checker = nullptr;

using simgrid::mc::remote;

#ifdef __linux__
# define WAITPID_CHECKED_FLAGS __WALL
#else
# define WAITPID_CHECKED_FLAGS 0
#endif

namespace simgrid {
namespace mc {

ModelChecker::ModelChecker(std::unique_ptr<RemoteClient> process)
    : base_(nullptr)
    , socket_event_(nullptr)
    , signal_event_(nullptr)
    , page_store_(500)
    , process_(std::move(process))
    , parent_snapshot_(nullptr)
{

}

ModelChecker::~ModelChecker() {
  if (socket_event_ != nullptr)
    event_free(socket_event_);
  if (signal_event_ != nullptr)
    event_free(signal_event_);
  if (base_ != nullptr)
    event_base_free(base_);
}

void ModelChecker::start()
{
  const pid_t pid = process_->pid();

  base_ = event_base_new();
  event_callback_fn event_callback = [](evutil_socket_t fd, short events, void *arg)
  {
    ((ModelChecker *)arg)->handle_events(fd, events);
  };
  socket_event_ = event_new(base_,
                            process_->getChannel().getSocket(),
                            EV_READ|EV_PERSIST,
                            event_callback, this);
  event_add(socket_event_, NULL);
  signal_event_ = event_new(base_,
                            SIGCHLD,
                            EV_SIGNAL|EV_PERSIST,
                            event_callback, this);
  event_add(signal_event_, NULL);

  XBT_DEBUG("Waiting for the model-checked process");
  int status;

  // The model-checked process SIGSTOP itself to signal it's ready:
  pid_t res = waitpid(pid, &status, WAITPID_CHECKED_FLAGS);
  if (res < 0 || not WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP)
    xbt_die("Could not wait model-checked process");

  process_->init();

  if (not _sg_mc_dot_output_file.get().empty())
    MC_init_dot_output();

  setup_ignore();

#ifdef __linux__
  ptrace(PTRACE_SETOPTIONS, pid, nullptr, PTRACE_O_TRACEEXIT);
  ptrace(PTRACE_CONT, pid, 0, 0);
#elif defined BSD
  ptrace(PT_CONTINUE, pid, (caddr_t)1, 0);
#else
# error "no ptrace equivalent coded for this platform"
#endif
}

static const std::pair<const char*, const char*> ignored_local_variables[] = {
  std::pair<const char*, const char*>{  "e", "*" },
  std::pair<const char*, const char*>{ "__ex_cleanup", "*" },
  std::pair<const char*, const char*>{ "__ex_mctx_en", "*" },
  std::pair<const char*, const char*>{ "__ex_mctx_me", "*" },
  std::pair<const char*, const char*>{ "__xbt_ex_ctx_ptr", "*" },
  std::pair<const char*, const char*>{ "_log_ev", "*" },
  std::pair<const char*, const char*>{ "_throw_ctx", "*" },
  std::pair<const char*, const char*>{ "ctx", "*" },

  std::pair<const char*, const char*>{ "self", "simcall_BODY_mc_snapshot" },
  std::pair<const char*, const char*>{ "next_context", "smx_ctx_sysv_suspend_serial" },
  std::pair<const char*, const char*>{ "i", "smx_ctx_sysv_suspend_serial" },

  /* Ignore local variable about time used for tracing */
  std::pair<const char*, const char*>{ "start_time", "*" },
};

void ModelChecker::setup_ignore()
{
  RemoteClient& process = this->process();
  for (std::pair<const char*, const char*> const& var :
      ignored_local_variables)
    process.ignore_local_variable(var.first, var.second);

  /* Static variable used for tracing */
  process.ignore_global_variable("counter");
}

void ModelChecker::shutdown()
{
  XBT_DEBUG("Shuting down model-checker");

  simgrid::mc::RemoteClient* process = &this->process();
  if (process->running()) {
    XBT_DEBUG("Killing process");
    kill(process->pid(), SIGKILL);
    process->terminate();
  }
}

void ModelChecker::resume(simgrid::mc::RemoteClient& process)
{
  int res = process.getChannel().send(MC_MESSAGE_CONTINUE);
  if (res)
    throw simgrid::xbt::errno_error();
  process.clear_cache();
}

static void MC_report_crash(int status)
{
  XBT_INFO("**************************");
  XBT_INFO("** CRASH IN THE PROGRAM **");
  XBT_INFO("**************************");
  if (WIFSIGNALED(status))
    XBT_INFO("From signal: %s", strsignal(WTERMSIG(status)));
  else if (WIFEXITED(status))
    XBT_INFO("From exit: %i", WEXITSTATUS(status));
  if (WCOREDUMP(status))
    XBT_INFO("A core dump was generated by the system.");
  else
    XBT_INFO("No core dump was generated by the system.");
  XBT_INFO("Counter-example execution trace:");
  simgrid::mc::dumpRecordPath();
  for (auto const& s : mc_model_checker->getChecker()->getTextualTrace())
    XBT_INFO("%s", s.c_str());
  simgrid::mc::session->logState();
  XBT_INFO("Stack trace:");
  mc_model_checker->process().dumpStack();
}

static void MC_report_assertion_error()
{
  XBT_INFO("**************************");
  XBT_INFO("*** PROPERTY NOT VALID ***");
  XBT_INFO("**************************");
  XBT_INFO("Counter-example execution trace:");
  simgrid::mc::dumpRecordPath();
  for (auto const& s : mc_model_checker->getChecker()->getTextualTrace())
    XBT_INFO("%s", s.c_str());
  simgrid::mc::session->logState();
}

bool ModelChecker::handle_message(char* buffer, ssize_t size)
{
  s_mc_message_t base_message;
  if (size < (ssize_t) sizeof(base_message))
    xbt_die("Broken message");
  memcpy(&base_message, buffer, sizeof(base_message));

  switch(base_message.type) {

  case MC_MESSAGE_IGNORE_HEAP:
    {
    s_mc_message_ignore_heap_t message;
    if (size != sizeof(message))
      xbt_die("Broken messsage");
    memcpy(&message, buffer, sizeof(message));

    IgnoredHeapRegion region;
    region.block    = message.block;
    region.fragment = message.fragment;
    region.address  = message.address;
    region.size     = message.size;
    process().ignore_heap(region);
    break;
    }

  case MC_MESSAGE_UNIGNORE_HEAP:
    {
    s_mc_message_ignore_memory_t message;
    if (size != sizeof(message))
      xbt_die("Broken messsage");
    memcpy(&message, buffer, sizeof(message));
    process().unignore_heap((void*)(std::uintptr_t)message.addr, message.size);
    break;
    }

  case MC_MESSAGE_IGNORE_MEMORY:
    {
    s_mc_message_ignore_memory_t message;
    if (size != sizeof(message))
      xbt_die("Broken messsage");
    memcpy(&message, buffer, sizeof(message));
    this->process().ignore_region(message.addr, message.size);
    break;
    }

  case MC_MESSAGE_STACK_REGION:
    {
    s_mc_message_stack_region_t message;
    if (size != sizeof(message))
      xbt_die("Broken messsage");
    memcpy(&message, buffer, sizeof(message));
    this->process().stack_areas().push_back(message.stack_region);
    }
    break;

  case MC_MESSAGE_REGISTER_SYMBOL:
    {
    s_mc_message_register_symbol_t message;
    if (size != sizeof(message))
      xbt_die("Broken message");
    memcpy(&message, buffer, sizeof(message));
    if (message.callback)
      xbt_die("Support for client-side function proposition is not implemented.");
    XBT_DEBUG("Received symbol: %s", message.name);

    if (simgrid::mc::property_automaton == nullptr)
      simgrid::mc::property_automaton = xbt_automaton_new();

    simgrid::mc::RemoteClient* process  = &this->process();
    simgrid::mc::RemotePtr<int> address = simgrid::mc::remote((int*)message.data);
    simgrid::xbt::add_proposition(simgrid::mc::property_automaton, message.name,
                                  [process, address]() { return process->read(address); });

    break;
    }

  case MC_MESSAGE_WAITING:
    return false;

  case MC_MESSAGE_ASSERTION_FAILED:
    MC_report_assertion_error();
    this->exit(SIMGRID_MC_EXIT_SAFETY);
    break;

  default:
    xbt_die("Unexpected message from model-checked application");

  }
  return true;
}

/** Terminate the model-checker application */
void ModelChecker::exit(int status)
{
  // TODO, terminate the model checker politely instead of exiting rudely
  if (process().running())
    kill(process().pid(), SIGKILL);
  ::exit(status);
}

void ModelChecker::handle_events(int fd, short events)
{
  if (events == EV_READ) {
    char buffer[MC_MESSAGE_LENGTH];
    ssize_t size = process_->getChannel().receive(buffer, sizeof(buffer), false);
    if (size == -1 && errno != EAGAIN)
      throw simgrid::xbt::errno_error();
    if (not handle_message(buffer, size)) {
      event_base_loopbreak(base_);
    }
  }
  else if (events == EV_SIGNAL) {
    on_signal(fd);
  }
  else {
    xbt_die("Unexpected event");
  }
}

void ModelChecker::loop()
{
  if (this->process().running())
    event_base_dispatch(base_);
}

void ModelChecker::handle_waitpid()
{
  XBT_DEBUG("Check for wait event");
  int status;
  pid_t pid;
  while ((pid = waitpid(-1, &status, WNOHANG)) != 0) {
    if (pid == -1) {
      if (errno == ECHILD) {
        // No more children:
        if (this->process().running())
          xbt_die("Inconsistent state");
        else
          break;
      } else {
        XBT_ERROR("Could not wait for pid");
        throw simgrid::xbt::errno_error();
      }
    }

    if (pid == this->process().pid()) {

      // From PTRACE_O_TRACEEXIT:
#ifdef __linux__
      if (status>>8 == (SIGTRAP | (PTRACE_EVENT_EXIT<<8))) {
        if (ptrace(PTRACE_GETEVENTMSG, this->process().pid(), 0, &status) == -1)
          xbt_die("Could not get exit status");
        if (WIFSIGNALED(status)) {
          MC_report_crash(status);
          mc_model_checker->exit(SIMGRID_MC_EXIT_PROGRAM_CRASH);
        }
      }
#endif

      // We don't care about signals, just reinject them:
      if (WIFSTOPPED(status)) {
        XBT_DEBUG("Stopped with signal %i", (int) WSTOPSIG(status));
        errno = 0;
#ifdef __linux__
        ptrace(PTRACE_CONT, this->process().pid(), 0, WSTOPSIG(status));
#elif defined BSD
        ptrace(PT_CONTINUE, this->process().pid(), (caddr_t)1, WSTOPSIG(status));
#endif
        if (errno != 0)
          xbt_die("Could not PTRACE_CONT");
      }

      else if (WIFEXITED(status) || WIFSIGNALED(status)) {
        XBT_DEBUG("Child process is over");
        this->process().terminate();
      }
    }
  }
}

void ModelChecker::on_signal(int signo)
{
  if (signo == SIGCHLD)
    this->handle_waitpid();
}

void ModelChecker::wait_for_requests()
{
  this->resume(process());
  if (this->process().running())
    event_base_dispatch(base_);
}

void ModelChecker::handle_simcall(Transition const& transition)
{
  s_mc_message_simcall_handle_t m;
  memset(&m, 0, sizeof(m));
  m.type  = MC_MESSAGE_SIMCALL_HANDLE;
  m.pid   = transition.pid;
  m.value = transition.argument;
  this->process_->getChannel().send(m);
  this->process_->clear_cache();
  if (this->process_->running())
    event_base_dispatch(base_);
}

bool ModelChecker::checkDeadlock()
{
  int res;
  if ((res = this->process().getChannel().send(MC_MESSAGE_DEADLOCK_CHECK)))
    xbt_die("Could not check deadlock state");
  s_mc_message_int_t message;
  ssize_t s = mc_model_checker->process().getChannel().receive(message);
  if (s == -1)
    xbt_die("Could not receive message");
  if (s != sizeof(message) || message.type != MC_MESSAGE_DEADLOCK_CHECK_REPLY)
    xbt_die("Received unexpected message %s (%i, size=%i) "
      "expected MC_MESSAGE_DEADLOCK_CHECK_REPLY (%i, size=%i)",
      MC_message_type_name(message.type), (int) message.type, (int) s,
      (int) MC_MESSAGE_DEADLOCK_CHECK_REPLY, (int) sizeof(message)
      );
  return message.value != 0;
}

}
}
