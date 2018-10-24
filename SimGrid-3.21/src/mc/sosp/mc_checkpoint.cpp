/* Copyright (c) 2008-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <unistd.h>

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <link.h>

#ifndef WIN32
#include <sys/mman.h>
#endif

#include "src/internal_config.h"
#include "src/mc/mc_private.hpp"
#include "src/smpi/include/private.hpp"
#include "xbt/file.hpp"
#include "xbt/mmalloc.h"
#include "xbt/module.h"

#include "src/xbt/mmalloc/mmprivate.h"

#include "src/simix/smx_private.hpp"

#include <libelf.h>
#include <libunwind.h>

#include "src/mc/mc_private.hpp"
#include <mc/mc.h>

#include "src/mc/mc_config.hpp"
#include "src/mc/mc_hash.hpp"
#include "src/mc/mc_mmu.hpp"
#include "src/mc/mc_smx.hpp"
#include "src/mc/mc_unw.hpp"
#include "src/mc/remote/mc_protocol.h"
#include "src/mc/sosp/mc_snapshot.hpp"

#include "src/mc/Frame.hpp"
#include "src/mc/ObjectInformation.hpp"
#include "src/mc/Variable.hpp"
#include "src/mc/sosp/RegionSnapshot.hpp"

using simgrid::mc::remote;

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(mc_checkpoint, mc, "Logging specific to mc_checkpoint");

#define PROT_RWX (PROT_READ | PROT_WRITE | PROT_EXEC)
#define PROT_RW (PROT_READ | PROT_WRITE)
#define PROT_RX (PROT_READ | PROT_EXEC)

namespace simgrid {
namespace mc {

/************************************  Free functions **************************************/
/*****************************************************************************************/

/** @brief Restore a region from a snapshot
 *
 *  @param region     Target region
 */
static void restore(mc_mem_region_t region)
{
  switch (region->storage_type()) {
    case simgrid::mc::StorageType::Flat:
      mc_model_checker->process().write_bytes(region->flat_data().get(), region->size(), region->permanent_address());
      break;

    case simgrid::mc::StorageType::Chunked:
      mc_region_restore_sparse(&mc_model_checker->process(), region);
      break;

    case simgrid::mc::StorageType::Privatized:
      for (auto& p : region->privatized_data())
        restore(&p);
      break;

    default: // includes StorageType::NoData
      xbt_die("Storage type not supported");
      break;
  }
}

#if HAVE_SMPI
RegionSnapshot privatized_region(RegionType region_type, void* start_addr, void* permanent_addr, std::size_t size)
{
  size_t process_count = MC_smpi_process_count();

  // Read smpi_privatization_regions from MCed:
  smpi_privatization_region_t remote_smpi_privatization_regions;
  mc_model_checker->process().read_variable("smpi_privatization_regions", &remote_smpi_privatization_regions,
                                            sizeof(remote_smpi_privatization_regions));
  s_smpi_privatization_region_t privatization_regions[process_count];
  mc_model_checker->process().read_bytes(&privatization_regions, sizeof(privatization_regions),
                                         remote(remote_smpi_privatization_regions));

  std::vector<simgrid::mc::RegionSnapshot> data;
  data.reserve(process_count);
  for (size_t i = 0; i < process_count; i++)
    data.push_back(simgrid::mc::region(region_type, start_addr, privatization_regions[i].address, size));

  simgrid::mc::RegionSnapshot region = simgrid::mc::RegionSnapshot(region_type, start_addr, permanent_addr, size);
  region.privatized_data(std::move(data));
  return region;
}
#endif

static void add_region(int index, simgrid::mc::Snapshot* snapshot, simgrid::mc::RegionType type,
                       simgrid::mc::ObjectInformation* object_info, void* start_addr, void* permanent_addr,
                       std::size_t size)
{
  if (type == simgrid::mc::RegionType::Data)
    xbt_assert(object_info, "Missing object info for object.");
  else if (type == simgrid::mc::RegionType::Heap)
    xbt_assert(not object_info, "Unexpected object info for heap region.");

  simgrid::mc::RegionSnapshot region;
#if HAVE_SMPI
  const bool privatization_aware = object_info && mc_model_checker->process().privatized(*object_info);
  if (privatization_aware && MC_smpi_process_count())
    region = simgrid::mc::privatized_region(type, start_addr, permanent_addr, size);
  else
#endif
    region = simgrid::mc::region(type, start_addr, permanent_addr, size);

  region.object_info(object_info);
  snapshot->snapshot_regions[index] =
      std::unique_ptr<simgrid::mc::RegionSnapshot>(new simgrid::mc::RegionSnapshot(std::move(region)));
}

static void get_memory_regions(simgrid::mc::RemoteClient* process, simgrid::mc::Snapshot* snapshot)
{
  const size_t n = process->object_infos.size();
  snapshot->snapshot_regions.resize(n + 1);
  int i = 0;
  for (auto const& object_info : process->object_infos)
    add_region(i++, snapshot, simgrid::mc::RegionType::Data, object_info.get(), object_info->start_rw,
               object_info->start_rw, object_info->end_rw - object_info->start_rw);

  xbt_mheap_t heap = process->get_heap();
  void* start_heap = heap->base;
  void* end_heap   = heap->breakval;

  add_region(n, snapshot, simgrid::mc::RegionType::Heap, nullptr, start_heap, start_heap,
             (char*)end_heap - (char*)start_heap);
  snapshot->heap_bytes_used = mmalloc_get_bytes_used_remote(heap->heaplimit, process->get_malloc_info());

#if HAVE_SMPI
  if (mc_model_checker->process().privatized() && MC_smpi_process_count())
    // snapshot->privatization_index = smpi_loaded_page
    mc_model_checker->process().read_variable("smpi_loaded_page", &snapshot->privatization_index,
                                              sizeof(snapshot->privatization_index));
  else
#endif
    snapshot->privatization_index = simgrid::mc::ProcessIndexMissing;
}

/** @brief Fills the position of the segments (executable, read-only, read/write).
 * */
// TODO, use the ELF segment information for more robustness
void find_object_address(std::vector<simgrid::xbt::VmMap> const& maps, simgrid::mc::ObjectInformation* result)
{
  std::string name = simgrid::xbt::Path(result->file_name).get_base_name();

  for (size_t i = 0; i < maps.size(); ++i) {
    simgrid::xbt::VmMap const& reg = maps[i];
    if (maps[i].pathname.empty())
      continue;
    std::string map_basename = simgrid::xbt::Path(maps[i].pathname).get_base_name();
    if (map_basename != name)
      continue;

    // This is the non-GNU_RELRO-part of the data segment:
    if (reg.prot == PROT_RW) {
      xbt_assert(not result->start_rw, "Multiple read-write segments for %s, not supported", maps[i].pathname.c_str());
      result->start_rw = (char*)reg.start_addr;
      result->end_rw   = (char*)reg.end_addr;

      // The next VMA might be end of the data segment:
      if (i + 1 < maps.size() && maps[i + 1].pathname.empty() && maps[i + 1].prot == PROT_RW &&
          maps[i + 1].start_addr == reg.end_addr)
        result->end_rw = (char*)maps[i + 1].end_addr;
    }

    // This is the text segment:
    else if (reg.prot == PROT_RX) {
      xbt_assert(not result->start_exec, "Multiple executable segments for %s, not supported",
                 maps[i].pathname.c_str());
      result->start_exec = (char*)reg.start_addr;
      result->end_exec   = (char*)reg.end_addr;

      // The next VMA might be end of the data segment:
      if (i + 1 < maps.size() && maps[i + 1].pathname.empty() && maps[i + 1].prot == PROT_RW &&
          maps[i + 1].start_addr == reg.end_addr) {
        result->start_rw = (char*)maps[i + 1].start_addr;
        result->end_rw   = (char*)maps[i + 1].end_addr;
      }
    }

    // This is the GNU_RELRO-part of the data segment:
    else if (reg.prot == PROT_READ) {
      xbt_assert(not result->start_ro, "Multiple read only segments for %s, not supported", maps[i].pathname.c_str());
      result->start_ro = (char*)reg.start_addr;
      result->end_ro   = (char*)reg.end_addr;
    }
  }

  result->start = result->start_rw;
  if ((const void*)result->start_ro < result->start)
    result->start = result->start_ro;
  if ((const void*)result->start_exec < result->start)
    result->start = result->start_exec;

  result->end = result->end_rw;
  if (result->end_ro && (const void*)result->end_ro > result->end)
    result->end = result->end_ro;
  if (result->end_exec && (const void*)result->end_exec > result->end)
    result->end = result->end_exec;

  xbt_assert(result->start_exec || result->start_rw || result->start_ro);
}

/************************************* Take Snapshot ************************************/
/****************************************************************************************/

/** @brief Checks whether the variable is in scope for a given IP.
 *
 *  A variable may be defined only from a given value of IP.
 *
 *  @param var   Variable description
 *  @param scope Scope description
 *  @param ip    Instruction pointer
 *  @return      true if the variable is valid
 * */
static bool valid_variable(simgrid::mc::Variable* var, simgrid::mc::Frame* scope, const void* ip)
{
  // The variable is not yet valid:
  if (scope->range.begin() + var->start_scope > (std::uint64_t)ip)
    return false;
  else
    return true;
}

static void fill_local_variables_values(mc_stack_frame_t stack_frame, simgrid::mc::Frame* scope, int process_index,
                                        std::vector<s_local_variable_t>& result)
{
  simgrid::mc::RemoteClient* process = &mc_model_checker->process();

  if (not scope || not scope->range.contain(stack_frame->ip))
    return;

  for (simgrid::mc::Variable& current_variable : scope->variables) {

    if (not valid_variable(&current_variable, scope, (void*)stack_frame->ip))
      continue;

    int region_type;
    // FIXME, get rid of `region_type`
    if ((long)stack_frame->ip > (long)process->libsimgrid_info->start_exec)
      region_type = 1;
    else
      region_type = 2;

    s_local_variable_t new_var;
    new_var.subprogram = stack_frame->frame;
    new_var.ip         = stack_frame->ip;
    new_var.name       = current_variable.name;
    new_var.type       = current_variable.type;
    new_var.region     = region_type;
    new_var.address    = nullptr;

    if (current_variable.address != nullptr)
      new_var.address = current_variable.address;
    else if (not current_variable.location_list.empty()) {
      simgrid::dwarf::Location location = simgrid::dwarf::resolve(
          current_variable.location_list, current_variable.object_info, &(stack_frame->unw_cursor),
          (void*)stack_frame->frame_base, &mc_model_checker->process(), process_index);

      if (not location.in_memory())
        xbt_die("Cannot handle non-address variable");
      new_var.address = location.address();

    } else
      xbt_die("No address");

    result.push_back(std::move(new_var));
  }

  // Recursive processing of nested scopes:
  for (simgrid::mc::Frame& nested_scope : scope->scopes)
    fill_local_variables_values(stack_frame, &nested_scope, process_index, result);
}

static std::vector<s_local_variable_t> get_local_variables_values(std::vector<s_mc_stack_frame_t>& stack_frames,
                                                                  int process_index)
{
  std::vector<s_local_variable_t> variables;
  for (s_mc_stack_frame_t& stack_frame : stack_frames)
    fill_local_variables_values(&stack_frame, stack_frame.frame, process_index, variables);
  return variables;
}

static std::vector<s_mc_stack_frame_t> unwind_stack_frames(simgrid::mc::UnwindContext* stack_context)
{
  simgrid::mc::RemoteClient* process = &mc_model_checker->process();
  std::vector<s_mc_stack_frame_t> result;

  unw_cursor_t c = stack_context->cursor();

  // TODO, check condition check (unw_init_local==0 means end of frame)

  while (1) {

    s_mc_stack_frame_t stack_frame;

    stack_frame.unw_cursor = c;

    unw_word_t ip;
    unw_word_t sp;

    unw_get_reg(&c, UNW_REG_IP, &ip);
    unw_get_reg(&c, UNW_REG_SP, &sp);

    stack_frame.ip = ip;
    stack_frame.sp = sp;

    // TODO, use real addresses in frame_t instead of fixing it here

    simgrid::mc::Frame* frame = process->find_function(remote(ip));
    stack_frame.frame         = frame;

    if (frame) {
      stack_frame.frame_name = frame->name;
      stack_frame.frame_base = (unw_word_t)frame->frame_base(c);
    } else {
      stack_frame.frame_base = 0;
      stack_frame.frame_name = std::string();
    }

    result.push_back(std::move(stack_frame));

    /* Stop before context switch with maestro */
    if (frame != nullptr && frame->name == "smx_ctx_sysv_wrapper")
      break;

    int ret = unw_step(&c);
    if (ret == 0)
      xbt_die("Unexpected end of stack.");
    else if (ret < 0)
      xbt_die("Error while unwinding stack");
  }

  if (result.empty()) {
    XBT_INFO("unw_init_local failed");
    xbt_abort();
  }

  return result;
}

static std::vector<s_mc_snapshot_stack_t> take_snapshot_stacks(simgrid::mc::Snapshot* snapshot)
{
  std::vector<s_mc_snapshot_stack_t> res;

  for (auto const& stack : mc_model_checker->process().stack_areas()) {
    s_mc_snapshot_stack_t st;

    // Read the context from remote process:
    unw_context_t context;
    mc_model_checker->process().read_bytes(&context, sizeof(context), remote(stack.context));

    st.context.initialize(&mc_model_checker->process(), &context);

    st.stack_frames    = unwind_stack_frames(&st.context);
    st.local_variables = get_local_variables_values(st.stack_frames, stack.process_index);
    st.process_index   = stack.process_index;

    unw_word_t sp = st.stack_frames[0].sp;

    res.push_back(std::move(st));

    size_t stack_size = (char*)stack.address + stack.size - (char*)sp;
    snapshot->stack_sizes.push_back(stack_size);
  }

  return res;
}

static void snapshot_handle_ignore(simgrid::mc::Snapshot* snapshot)
{
  xbt_assert(snapshot->process());

  // Copy the memory:
  for (auto const& region : mc_model_checker->process().ignored_regions()) {
    s_mc_snapshot_ignored_data_t ignored_data;
    ignored_data.start = (void*)region.addr;
    ignored_data.data.resize(region.size);
    // TODO, we should do this once per privatization segment:
    snapshot->process()->read_bytes(ignored_data.data.data(), region.size, remote(region.addr),
                                    simgrid::mc::ProcessIndexDisabled);
    snapshot->ignored_data.push_back(std::move(ignored_data));
  }

  // Zero the memory:
  for (auto const& region : mc_model_checker->process().ignored_regions())
    snapshot->process()->clear_bytes(remote(region.addr), region.size);
}

static void snapshot_ignore_restore(simgrid::mc::Snapshot* snapshot)
{
  for (auto const& ignored_data : snapshot->ignored_data)
    snapshot->process()->write_bytes(ignored_data.data.data(), ignored_data.data.size(), remote(ignored_data.start));
}

static std::vector<s_fd_infos_t> get_current_fds(pid_t pid)
{
  const size_t fd_dir_path_size = 20;
  char fd_dir_path[fd_dir_path_size];
  int res = snprintf(fd_dir_path, fd_dir_path_size, "/proc/%lli/fd", (long long int)pid);
  xbt_assert(res >= 0);
  if ((size_t)res > fd_dir_path_size)
    xbt_die("Unexpected buffer is too small for fd_dir_path");

  DIR* fd_dir = opendir(fd_dir_path);
  if (fd_dir == nullptr)
    xbt_die("Cannot open directory '/proc/self/fd'\n");

  std::vector<s_fd_infos_t> fds;

  struct dirent* fd_number;
  while ((fd_number = readdir(fd_dir))) {

    int fd_value = xbt_str_parse_int(fd_number->d_name, "Found a non-numerical FD: %s. Freaking out!");

    if (fd_value < 3)
      continue;

    const size_t source_size = 25;
    char source[25];
    int res = snprintf(source, source_size, "/proc/%lli/fd/%s", (long long int)pid, fd_number->d_name);
    xbt_assert(res >= 0);
    if ((size_t)res > source_size)
      xbt_die("Unexpected buffer is too small for fd %s", fd_number->d_name);

    const size_t link_size = 200;
    char link[200];
    res = readlink(source, link, link_size);

    if (res < 0)
      xbt_die("Could not read link for %s", source);
    if (res == 200)
      xbt_die("Buffer to small for link of %s", source);

    link[res] = '\0';

#if HAVE_SMPI
    if (smpi_is_privatization_file(link))
      continue;
#endif

    // This is (probably) the DIR* we are reading:
    // TODO, read all the file entries at once and close the DIR.*
    if (strcmp(fd_dir_path, link) == 0)
      continue;

    // We don't handle them.
    // It does not mean we should silently ignore them however.
    if (strncmp(link, "pipe:", std::strlen("pipe:")) == 0 || strncmp(link, "socket:", std::strlen("socket:")) == 0)
      continue;

    // If dot_output enabled, do not handle the corresponding file
    if (dot_output != nullptr) {
      std::string link_basename = simgrid::xbt::Path(link).get_base_name();
      if (link_basename == _sg_mc_dot_output_file.get())
        continue;
    }

    // This is probably a shared memory used by lttng-ust:
    if (strncmp("/dev/shm/ust-shm-tmp-", link, std::strlen("/dev/shm/ust-shm-tmp-")) == 0)
      continue;

    // Add an entry for this FD in the snapshot:
    s_fd_infos_t fd;
    fd.filename         = std::string(link);
    fd.number           = fd_value;
    fd.flags            = fcntl(fd_value, F_GETFL) | fcntl(fd_value, F_GETFD);
    fd.current_position = lseek(fd_value, 0, SEEK_CUR);
    fds.push_back(std::move(fd));
  }

  closedir(fd_dir);
  return fds;
}

std::shared_ptr<simgrid::mc::Snapshot> take_snapshot(int num_state)
{
  XBT_DEBUG("Taking snapshot %i", num_state);

  simgrid::mc::RemoteClient* mc_process = &mc_model_checker->process();

  std::shared_ptr<simgrid::mc::Snapshot> snapshot = std::make_shared<simgrid::mc::Snapshot>(mc_process, num_state);

  for (auto const& p : mc_model_checker->process().actors())
    snapshot->enabled_processes.insert(p.copy.getBuffer()->pid_);

  snapshot_handle_ignore(snapshot.get());

  if (_sg_mc_snapshot_fds)
    snapshot->current_fds = get_current_fds(mc_model_checker->process().pid());

  /* Save the std heap and the writable mapped pages of libsimgrid and binary */
  get_memory_regions(mc_process, snapshot.get());

  snapshot->to_ignore = mc_model_checker->process().ignored_heap();

  if (_sg_mc_max_visited_states > 0 || not _sg_mc_property_file.get().empty()) {
    snapshot->stacks = take_snapshot_stacks(snapshot.get());
    if (_sg_mc_hash)
      snapshot->hash = simgrid::mc::hash(*snapshot);
    else
      snapshot->hash = 0;
  } else
    snapshot->hash = 0;

  snapshot_ignore_restore(snapshot.get());
  return snapshot;
}

static inline void restore_snapshot_regions(simgrid::mc::Snapshot* snapshot)
{
  for (std::unique_ptr<s_mc_mem_region_t> const& region : snapshot->snapshot_regions) {
    // For privatized, variables we decided it was not necessary to take the snapshot:
    if (region)
      restore(region.get());
  }

#if HAVE_SMPI
  if (snapshot->privatization_index >= 0) {
    // Fix the privatization mmap:
    s_mc_message_restore_t message{MC_MESSAGE_RESTORE, snapshot->privatization_index};
    mc_model_checker->process().getChannel().send(message);
  }
#endif
}

static inline void restore_snapshot_fds(simgrid::mc::Snapshot* snapshot)
{
  xbt_die("FD snapshot not implemented in client/server mode.");

  for (auto const& fd : snapshot->current_fds) {

    int new_fd = open(fd.filename.c_str(), fd.flags);
    if (new_fd < 0)
      xbt_die("Could not reopen the file %s fo restoring the file descriptor", fd.filename.c_str());
    if (new_fd != fd.number) {
      dup2(new_fd, fd.number);
      close(new_fd);
    }
    lseek(fd.number, fd.current_position, SEEK_SET);
  }
}

void restore_snapshot(std::shared_ptr<simgrid::mc::Snapshot> snapshot)
{
  XBT_DEBUG("Restore snapshot %i", snapshot->num_state);
  restore_snapshot_regions(snapshot.get());
  if (_sg_mc_snapshot_fds)
    restore_snapshot_fds(snapshot.get());
  snapshot_ignore_restore(snapshot.get());
  mc_model_checker->process().clear_cache();
}

} // namespace mc
} // namespace simgrid
