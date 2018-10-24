/* Copyright (c) 2009-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_SIMIX_UNIX_CONTEXT_HPP
#define SIMGRID_SIMIX_UNIX_CONTEXT_HPP

#include <ucontext.h> /* context relative declarations */

#include <atomic>
#include <cstdint>
#include <functional>
#include <vector>

#include <simgrid/simix.hpp>
#include <xbt/parmap.hpp>
#include <xbt/xbt_os_thread.h>

#include "Context.hpp"
#include "src/internal_config.h"
#include "src/simix/smx_private.hpp"

namespace simgrid {
namespace kernel {
namespace context {

class UContext : public Context {
public:
  UContext(std::function<void()> code, void_pfn_smxprocess_t cleanup_func, smx_actor_t process);
  ~UContext() override;
  void stop() override;
  virtual void resume() = 0;

  static void swap(UContext* from, UContext* to);
  static UContext* getMaestro() { return maestro_context_; }
  static void setMaestro(UContext* maestro) { maestro_context_ = maestro; }

private:
  static UContext* maestro_context_;
  void* stack_ = nullptr; /* the thread stack */
  ucontext_t uc_;         /* the ucontext that executes the code */

#if HAVE_SANITIZER_ADDRESS_FIBER_SUPPORT
  const void* asan_stack_ = nullptr;
  size_t asan_stack_size_ = 0;
  UContext* asan_ctx_     = nullptr;
  bool asan_stop_         = false;
#endif

  static void smx_ctx_sysv_wrapper(int, int);
  static void make_ctx(ucontext_t* ucp, void (*func)(int, int), UContext* arg);
};

class SerialUContext : public UContext {
public:
  SerialUContext(std::function<void()> code, void_pfn_smxprocess_t cleanup_func, smx_actor_t process)
      : UContext(std::move(code), cleanup_func, process)
  {
  }
  void suspend() override;
  void resume() override;

  static void run_all();

private:
  static unsigned long process_index_;
};

#if HAVE_THREAD_CONTEXTS
class ParallelUContext : public UContext {
public:
  ParallelUContext(std::function<void()> code, void_pfn_smxprocess_t cleanup_func, smx_actor_t process)
      : UContext(std::move(code), cleanup_func, process)
  {
  }
  void suspend() override;
  void resume() override;

  static void initialize();
  static void finalize();
  static void run_all();

private:
  static simgrid::xbt::Parmap<smx_actor_t>* parmap_;
  static std::vector<ParallelUContext*> workers_context_;
  static std::atomic<uintptr_t> threads_working_;
  static thread_local uintptr_t worker_id_;
};
#endif

class UContextFactory : public ContextFactory {
public:
  UContextFactory();
  ~UContextFactory() override;
  Context* create_context(std::function<void()> code, void_pfn_smxprocess_t cleanup, smx_actor_t process) override;
  void run_all() override;

private:
  bool parallel_;
};
}}} // namespace

#endif
