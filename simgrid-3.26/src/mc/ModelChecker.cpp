/* Copyright (c) 2008-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "src/mc/ModelChecker.hpp"
#include "src/mc/Session.hpp"
#include "src/mc/Transition.hpp"
#include "src/mc/checker/Checker.hpp"
#include "src/mc/mc_config.hpp"
#include "src/mc/mc_exit.hpp"
#include "src/mc/mc_private.hpp"
#include "src/mc/remote/RemoteSimulation.hpp"
#include "xbt/automaton.hpp"
#include "xbt/system_error.hpp"

#include <array>
#include <sys/ptrace.h>
#include <sys/wait.h>

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

ModelChecker::ModelChecker(std::unique_ptr<RemoteSimulation> remote_simulation, int sockfd)
    : checker_side_(sockfd), remote_simulation_(std::move(remote_simulation))
{
}

void ModelChecker::start()
{
  checker_side_.start([](evutil_socket_t sig, short events, void* arg) {
    auto mc = static_cast<simgrid::mc::ModelChecker*>(arg);
    if (events == EV_READ) {
      std::array<char, MC_MESSAGE_LENGTH> buffer;
      ssize_t size = mc->checker_side_.get_channel().receive(buffer.data(), buffer.size(), false);
      if (size == -1 && errno != EAGAIN)
        throw simgrid::xbt::errno_error();

      if (not mc->handle_message(buffer.data(), size))
        mc->checker_side_.break_loop();
    } else if (events == EV_SIGNAL) {
      if (sig == SIGCHLD)
        mc->handle_waitpid();
    } else {
      xbt_die("Unexpected event");
    }
  });

  XBT_DEBUG("Waiting for the model-checked process");
  int status;

  // The model-checked process SIGSTOP itself to signal it's ready:
  const pid_t pid = remote_simulation_->pid();

  pid_t res = waitpid(pid, &status, WAITPID_CHECKED_FLAGS);
  if (res < 0 || not WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP)
    xbt_die("Could not wait model-checked process");

  remote_simulation_->init();

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

static constexpr auto ignored_local_variables = {
    std::make_pair("e", "*"),
    std::make_pair("_log_ev", "*"),

    /* Ignore local variable about time used for tracing */
    std::make_pair("start_time", "*"),
};

void ModelChecker::setup_ignore()
{
  const RemoteSimulation& process = this->get_remote_simulation();
  for (auto const& var : ignored_local_variables)
    process.ignore_local_variable(var.first, var.second);

  /* Static variable used for tracing */
  process.ignore_global_variable("counter");
}

void ModelChecker::shutdown()
{
  XBT_DEBUG("Shutting down model-checker");

  RemoteSimulation* process = &this->get_remote_simulation();
  if (process->running()) {
    XBT_DEBUG("Killing process");
    kill(process->pid(), SIGKILL);
    process->terminate();
  }
}

void ModelChecker::resume(RemoteSimulation& process)
{
  int res = checker_side_.get_channel().send(MessageType::CONTINUE);
  if (res)
    throw xbt::errno_error();
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
  if (not xbt_log_no_loc)
    XBT_INFO("%s core dump was generated by the system.", WCOREDUMP(status) ? "A" : "No");
  XBT_INFO("Counter-example execution trace:");
  for (auto const& s : mc_model_checker->getChecker()->get_textual_trace())
    XBT_INFO("  %s", s.c_str());
  dumpRecordPath();
  session->log_state();
  if (xbt_log_no_loc) {
    XBT_INFO("Stack trace not displayed because you passed --log=no_loc");
  } else {
    XBT_INFO("Stack trace:");
    mc_model_checker->get_remote_simulation().dump_stack();
  }
}

static void MC_report_assertion_error()
{
  XBT_INFO("**************************");
  XBT_INFO("*** PROPERTY NOT VALID ***");
  XBT_INFO("**************************");
  XBT_INFO("Counter-example execution trace:");
  for (auto const& s : mc_model_checker->getChecker()->get_textual_trace())
    XBT_INFO("  %s", s.c_str());
  dumpRecordPath();
  session->log_state();
}

bool ModelChecker::handle_message(const char* buffer, ssize_t size)
{
  s_mc_message_t base_message;
  xbt_assert(size >= (ssize_t)sizeof(base_message), "Broken message");
  memcpy(&base_message, buffer, sizeof(base_message));

  switch(base_message.type) {
    case MessageType::IGNORE_HEAP: {
      s_mc_message_ignore_heap_t message;
      xbt_assert(size == sizeof(message), "Broken message");
      memcpy(&message, buffer, sizeof(message));

      IgnoredHeapRegion region;
      region.block    = message.block;
      region.fragment = message.fragment;
      region.address  = message.address;
      region.size     = message.size;
      get_remote_simulation().ignore_heap(region);
      break;
    }

    case MessageType::UNIGNORE_HEAP: {
      s_mc_message_ignore_memory_t message;
      xbt_assert(size == sizeof(message), "Broken message");
      memcpy(&message, buffer, sizeof(message));
      get_remote_simulation().unignore_heap((void*)(std::uintptr_t)message.addr, message.size);
      break;
    }

    case MessageType::IGNORE_MEMORY: {
      s_mc_message_ignore_memory_t message;
      xbt_assert(size == sizeof(message), "Broken message");
      memcpy(&message, buffer, sizeof(message));
      this->get_remote_simulation().ignore_region(message.addr, message.size);
      break;
    }

    case MessageType::STACK_REGION: {
      s_mc_message_stack_region_t message;
      xbt_assert(size == sizeof(message), "Broken message");
      memcpy(&message, buffer, sizeof(message));
      this->get_remote_simulation().stack_areas().push_back(message.stack_region);
    } break;

    case MessageType::REGISTER_SYMBOL: {
      s_mc_message_register_symbol_t message;
      xbt_assert(size == sizeof(message), "Broken message");
      memcpy(&message, buffer, sizeof(message));
      xbt_assert(not message.callback, "Support for client-side function proposition is not implemented.");
      XBT_DEBUG("Received symbol: %s", message.name.data());

      if (property_automaton == nullptr)
        property_automaton = xbt_automaton_new();

      const RemoteSimulation* process = &this->get_remote_simulation();
      RemotePtr<int> address          = remote((int*)message.data);
      xbt::add_proposition(property_automaton, message.name.data(),
                           [process, address]() { return process->read(address); });

      break;
    }

    case MessageType::WAITING:
      return false;

    case MessageType::ASSERTION_FAILED:
      MC_report_assertion_error();
      this->exit(SIMGRID_MC_EXIT_SAFETY);

    default:
      xbt_die("Unexpected message from model-checked application");
  }
  return true;
}

/** Terminate the model-checker application */
void ModelChecker::exit(int status)
{
  // TODO, terminate the model checker politely instead of exiting rudely
  if (get_remote_simulation().running())
    kill(get_remote_simulation().pid(), SIGKILL);
  ::exit(status);
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
        xbt_assert(not this->get_remote_simulation().running(), "Inconsistent state");
        break;
      } else {
        XBT_ERROR("Could not wait for pid");
        throw simgrid::xbt::errno_error();
      }
    }

    if (pid == this->get_remote_simulation().pid()) {
      // From PTRACE_O_TRACEEXIT:
#ifdef __linux__
      if (status>>8 == (SIGTRAP | (PTRACE_EVENT_EXIT<<8))) {
        xbt_assert(ptrace(PTRACE_GETEVENTMSG, remote_simulation_->pid(), 0, &status) != -1,
                   "Could not get exit status");
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
        ptrace(PTRACE_CONT, remote_simulation_->pid(), 0, WSTOPSIG(status));
#elif defined BSD
        ptrace(PT_CONTINUE, remote_simulation_->pid(), (caddr_t)1, WSTOPSIG(status));
#endif
        xbt_assert(errno == 0, "Could not PTRACE_CONT");
      }

      else if (WIFSIGNALED(status)) {
        MC_report_crash(status);
        mc_model_checker->exit(SIMGRID_MC_EXIT_PROGRAM_CRASH);
      } else if (WIFEXITED(status)) {
        XBT_DEBUG("Child process is over");
        this->get_remote_simulation().terminate();
      }
    }
  }
}

void ModelChecker::wait_for_requests()
{
  this->resume(get_remote_simulation());
  if (this->get_remote_simulation().running())
    checker_side_.dispatch();
}

void ModelChecker::handle_simcall(Transition const& transition)
{
  s_mc_message_simcall_handle_t m;
  memset(&m, 0, sizeof(m));
  m.type  = MessageType::SIMCALL_HANDLE;
  m.pid   = transition.pid_;
  m.value = transition.argument_;
  checker_side_.get_channel().send(m);
  this->remote_simulation_->clear_cache();
  if (this->remote_simulation_->running())
    checker_side_.dispatch();
}

bool ModelChecker::checkDeadlock()
{
  int res = checker_side_.get_channel().send(MessageType::DEADLOCK_CHECK);
  xbt_assert(res == 0, "Could not check deadlock state");
  s_mc_message_int_t message;
  ssize_t s = checker_side_.get_channel().receive(message);
  xbt_assert(s != -1, "Could not receive message");
  xbt_assert(s == sizeof(message) && message.type == MessageType::DEADLOCK_CHECK_REPLY,
             "Received unexpected message %s (%i, size=%i) "
             "expected MessageType::DEADLOCK_CHECK_REPLY (%i, size=%i)",
             MC_message_type_name(message.type), (int)message.type, (int)s, (int)MessageType::DEADLOCK_CHECK_REPLY,
             (int)sizeof(message));
  return message.value != 0;
}

} // namespace mc
} // namespace simgrid