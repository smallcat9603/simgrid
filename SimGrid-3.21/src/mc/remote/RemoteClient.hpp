/* Copyright (c) 2008-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_MC_PROCESS_H
#define SIMGRID_MC_PROCESS_H

#include "src/xbt/mmalloc/mmprivate.h"
#include "src/mc/remote/Channel.hpp"
#include "src/mc/ObjectInformation.hpp"

#include <vector>

namespace simgrid {
namespace mc {

class ActorInformation {
public:
  /** MCed address of the process */
  RemotePtr<simgrid::kernel::actor::ActorImpl> address{nullptr};
  Remote<simgrid::kernel::actor::ActorImpl> copy;

  /** Hostname (owned by `mc_modelchecker->hostnames`) */
  const char* hostname = nullptr;
  std::string name;

  void clear()
  {
    name.clear();
    address  = nullptr;
    hostname = nullptr;
  }
};

struct IgnoredRegion {
  std::uint64_t addr;
  std::size_t size;
};

struct IgnoredHeapRegion {
  int block;
  int fragment;
  void* address;
  std::size_t size;
};

/** The Model-Checked process, seen from the MCer perspective
 *
 *  This class is mixing a lot of different responsibilities and is tied
 *  to SIMIX. It should probably be split into different classes.
 *
 *  Responsibilities:
 *
 *  - reading from the process memory (`AddressSpace`);
 *  - accessing the system state of the process (heap, …);
 *  - storing the SIMIX state of the process;
 *  - privatization;
 *  - communication with the model-checked process;
 *  - stack unwinding;
 *  - etc.
 */
class RemoteClient final : public AddressSpace {
private:
  // Those flags are used to track down which cached information
  // is still up to date and which information needs to be updated.
  static constexpr int cache_none            = 0;
  static constexpr int cache_heap            = 1;
  static constexpr int cache_malloc          = 2;
  static constexpr int cache_simix_processes = 4;

public:
  RemoteClient(pid_t pid, int sockfd);
  ~RemoteClient();
  void init();

  RemoteClient(RemoteClient const&) = delete;
  RemoteClient(RemoteClient&&)      = delete;
  RemoteClient& operator=(RemoteClient const&) = delete;
  RemoteClient& operator=(RemoteClient&&) = delete;

  // Read memory:
  const void* read_bytes(void* buffer, std::size_t size, RemotePtr<void> address, int process_index = ProcessIndexAny,
                         ReadOptions options = ReadOptions::none()) const override;

  void read_variable(const char* name, void* target, size_t size) const;
  template <class T> void read_variable(const char* name, T* target) const
  {
    read_variable(name, target, sizeof(*target));
  }
  template <class T> Remote<T> read_variable(const char* name) const
  {
    Remote<T> res;
    read_variable(name, res.getBuffer(), sizeof(T));
    return res;
  }

  std::string read_string(RemotePtr<char> address) const;
  using AddressSpace::read_string;

  // Write memory:
  void write_bytes(const void* buffer, size_t len, RemotePtr<void> address);
  void clear_bytes(RemotePtr<void> address, size_t len);

  // Debug information:
  std::shared_ptr<simgrid::mc::ObjectInformation> find_object_info(RemotePtr<void> addr) const;
  std::shared_ptr<simgrid::mc::ObjectInformation> find_object_info_exec(RemotePtr<void> addr) const;
  std::shared_ptr<simgrid::mc::ObjectInformation> find_object_info_rw(RemotePtr<void> addr) const;
  simgrid::mc::Frame* find_function(RemotePtr<void> ip) const;
  simgrid::mc::Variable* find_variable(const char* name) const;

  // Heap access:
  xbt_mheap_t get_heap()
  {
    if (not(this->cache_flags_ & RemoteClient::cache_heap))
      this->refresh_heap();
    return this->heap.get();
  }
  const malloc_info* get_malloc_info()
  {
    if (not(this->cache_flags_ & RemoteClient::cache_malloc))
      this->refresh_malloc_info();
    return this->heap_info.data();
  }

  void clear_cache() { this->cache_flags_ = RemoteClient::cache_none; }

  Channel const& getChannel() const { return channel_; }
  Channel& getChannel() { return channel_; }

  std::vector<IgnoredRegion> const& ignored_regions() const { return ignored_regions_; }
  void ignore_region(std::uint64_t address, std::size_t size);

  pid_t pid() const { return pid_; }

  bool in_maestro_stack(RemotePtr<void> p) const
  {
    return p >= this->maestro_stack_start_ && p < this->maestro_stack_end_;
  }

  bool running() const { return running_; }

  void terminate() { running_ = false; }

  bool privatized(ObjectInformation const& info) const { return privatized_ && info.executable(); }
  bool privatized() const { return privatized_; }
  void privatized(bool privatized) { privatized_ = privatized; }

  void ignore_global_variable(const char* name)
  {
    for (std::shared_ptr<simgrid::mc::ObjectInformation> const& info : this->object_infos)
      info->remove_global_variable(name);
  }

  std::vector<s_stack_region_t>& stack_areas() { return stack_areas_; }
  std::vector<s_stack_region_t> const& stack_areas() const { return stack_areas_; }

  std::vector<IgnoredHeapRegion> const& ignored_heap() const { return ignored_heap_; }
  void ignore_heap(IgnoredHeapRegion const& region);
  void unignore_heap(void* address, size_t size);

  void ignore_local_variable(const char* var_name, const char* frame_name);
  std::vector<simgrid::mc::ActorInformation>& actors();
  std::vector<simgrid::mc::ActorInformation>& dead_actors();

  /** Get a local description of a remote SIMIX actor */
  simgrid::mc::ActorInformation* resolveActorInfo(simgrid::mc::RemotePtr<simgrid::kernel::actor::ActorImpl> actor)
  {
    xbt_assert(mc_model_checker != nullptr);
    if (not actor)
      return nullptr;
    this->refresh_simix();
    for (auto& actor_info : this->smx_actors_infos)
      if (actor_info.address == actor)
        return &actor_info;
    for (auto& actor_info : this->smx_dead_actors_infos)
      if (actor_info.address == actor)
        return &actor_info;
    return nullptr;
  }

  /** Get a local copy of the SIMIX actor structure */
  simgrid::kernel::actor::ActorImpl* resolveActor(simgrid::mc::RemotePtr<simgrid::kernel::actor::ActorImpl> process)
  {
    simgrid::mc::ActorInformation* actor_info = this->resolveActorInfo(process);
    if (actor_info)
      return actor_info->copy.getBuffer();
    else
      return nullptr;
  }

  void dumpStack();

private:
  void init_memory_map_info();
  void refresh_heap();
  void refresh_malloc_info();
  void refresh_simix();

  pid_t pid_ = -1;
  Channel channel_;
  bool running_ = false;
  std::vector<simgrid::xbt::VmMap> memory_map_;
  RemotePtr<void> maestro_stack_start_;
  RemotePtr<void> maestro_stack_end_;
  int memory_file = -1;
  std::vector<IgnoredRegion> ignored_regions_;
  bool privatized_ = false;
  std::vector<s_stack_region_t> stack_areas_;
  std::vector<IgnoredHeapRegion> ignored_heap_;

public:
  // object info
  // TODO, make private (first, objectify simgrid::mc::ObjectInformation*)
  std::vector<std::shared_ptr<simgrid::mc::ObjectInformation>> object_infos;
  std::shared_ptr<simgrid::mc::ObjectInformation> libsimgrid_info;
  std::shared_ptr<simgrid::mc::ObjectInformation> binary_info;

  // Copies of MCed SMX data structures
  /** Copy of `simix_global->process_list`
   *
   *  See mc_smx.c.
   */
  std::vector<ActorInformation> smx_actors_infos;

  /** Copy of `simix_global->process_to_destroy`
   *
   *  See mc_smx.c.
   */
  std::vector<ActorInformation> smx_dead_actors_infos;

private:
  /** State of the cache (which variables are up to date) */
  int cache_flags_ = RemoteClient::cache_none;

public:
  /** Address of the heap structure in the MCed process. */
  void* heap_address;

  /** Copy of the heap structure of the process
   *
   *  This is refreshed with the `MC_process_refresh` call.
   *  This is not used if the process is the current one:
   *  use `get_heap_info()` in order to use it.
   */
  std::unique_ptr<s_xbt_mheap_t> heap;

  /** Copy of the allocation info structure
   *
   *  This is refreshed with the `MC_process_refresh` call.
   *  This is not used if the process is the current one:
   *  use `get_malloc_info()` in order to use it.
   */
  std::vector<malloc_info> heap_info;

  // Libunwind-data
  /** Full-featured MC-aware libunwind address space for the process
   *
   *  This address space is using a simgrid::mc::UnwindContext*
   *  (with simgrid::mc::Process* / simgrid::mc::AddressSpace*
   *  and unw_context_t).
   */
  unw_addr_space_t unw_addr_space;

  /** Underlying libunwind address-space
   *
   *  The `find_proc_info`, `put_unwind_info`, `get_dyn_info_list_addr`
   *  operations of the native MC address space is currently delegated
   *  to this address space (either the local or a ptrace unwinder).
   */
  unw_addr_space_t unw_underlying_addr_space;

  /** The corresponding context
   */
  void* unw_underlying_context;

  /* Check whether the given actor is enabled */
  bool actor_is_enabled(aid_t pid);
};

/** Open a FD to a remote process memory (`/dev/$pid/mem`)
 */
XBT_PRIVATE int open_vm(pid_t pid, int flags);
}
}

#endif
