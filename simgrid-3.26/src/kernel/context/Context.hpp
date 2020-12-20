/* Copyright (c) 2007-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_KERNEL_CONTEXT_CONTEXT_HPP
#define SIMGRID_KERNEL_CONTEXT_CONTEXT_HPP

#include "simgrid/forward.h"
#include "src/kernel/activity/ActivityImpl.hpp"

#include <csignal>
#include <functional>

namespace simgrid {
namespace kernel {
namespace context {

class XBT_PUBLIC ContextFactory {
public:
  explicit ContextFactory()             = default;
  ContextFactory(const ContextFactory&) = delete;
  ContextFactory& operator=(const ContextFactory&) = delete;
  virtual ~ContextFactory();
  virtual Context* create_context(std::function<void()>&& code, actor::ActorImpl* actor) = 0;

  /** Turn the current thread into a simulation context */
  virtual Context* attach(actor::ActorImpl* actor);
  /** Turn the current thread into maestro (the old maestro becomes a regular actor) */
  virtual Context* create_maestro(std::function<void()>&& code, actor::ActorImpl* actor);

  virtual void run_all() = 0;

protected:
  template <class T, class... Args> T* new_context(Args&&... args)
  {
    auto* context = new T(std::forward<Args>(args)...);
    context->declare_context(sizeof(T));
    return context;
  }
};

class XBT_PUBLIC Context {
  friend ContextFactory;

  static thread_local Context* current_context_;

  std::function<void()> code_;
  actor::ActorImpl* actor_ = nullptr;
  bool iwannadie_          = false;
  void declare_context(std::size_t size);

public:
#ifndef WIN32
  static int install_sigsegv_stack(stack_t* old_stack, bool enable);
#endif

  Context(std::function<void()>&& code, actor::ActorImpl* actor);
  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;
  virtual ~Context();

  bool wannadie() const { return iwannadie_; }
  void set_wannadie(bool value = true) { iwannadie_ = value; }
  void operator()() const { code_(); }
  bool has_code() const { return static_cast<bool>(code_); }
  actor::ActorImpl* get_actor() const { return this->actor_; }

  // Scheduling methods
  virtual void stop();
  virtual void suspend() = 0;

  // Retrieving the self() context
  /** @brief Retrieves the current context of this thread */
  static Context* self();
  /** @brief Sets the current context of this thread */
  static void set_current(Context* self);
};

class XBT_PUBLIC AttachContext : public Context {
public:
  AttachContext(std::function<void()>&& code, actor::ActorImpl* actor) : Context(std::move(code), actor) {}
  AttachContext(const AttachContext&) = delete;
  AttachContext& operator=(const AttachContext&) = delete;
  ~AttachContext() override;

  /** Called by the context when it is ready to give control
   *  to the maestro.
   */
  virtual void attach_start() = 0;

  /** Called by the context when it has finished its job */
  virtual void attach_stop() = 0;
};


/* This allows Java to hijack the context factory (Java induces factories of factory :) */
using ContextFactoryInitializer = ContextFactory* (*)();
XBT_PUBLIC_DATA ContextFactoryInitializer factory_initializer;

XBT_PRIVATE ContextFactory* thread_factory();
XBT_PRIVATE ContextFactory* sysv_factory();
XBT_PRIVATE ContextFactory* raw_factory();
XBT_PRIVATE ContextFactory* boost_factory();

} // namespace context
} // namespace kernel
} // namespace simgrid

XBT_PRIVATE void SIMIX_context_mod_init();
XBT_PRIVATE void SIMIX_context_mod_exit();
#endif
