/* Copyright (c) 2015-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/s4u/Host.hpp"

#include "src/mc/ModelChecker.hpp"
#include "src/mc/mc_smx.hpp"

using simgrid::mc::remote;

/** HACK, Statically "upcast" a s_smx_actor_t into a ActorInformation
 *
 *  This gets 'actorInfo' from '&actorInfo->copy'. It upcasts in the
 *  sense that we could achieve the same thing by having ActorInformation
 *  inherit from s_smx_actor_t but we don't really want to do that.
 */
static inline simgrid::mc::ActorInformation* actor_info_cast(smx_actor_t actor)
{
  simgrid::mc::ActorInformation temp;
  std::size_t offset = (char*) temp.copy.getBuffer() - (char*)&temp;

  simgrid::mc::ActorInformation* process_info = (simgrid::mc::ActorInformation*)((char*)actor - offset);
  return process_info;
}

/** Load the remote list of processes into a vector
 *
 *  @param process      MCed process
 *  @param target       Local vector (to be filled with copies of `s_smx_actor_t`)
 *  @param remote_dynar Address of the process dynar in the remote list
 */
static void MC_process_refresh_simix_actor_dynar(simgrid::mc::RemoteClient* process,
                                                 std::vector<simgrid::mc::ActorInformation>& target,
                                                 simgrid::mc::RemotePtr<s_xbt_dynar_t> remote_dynar)
{
  target.clear();

  s_xbt_dynar_t dynar;
  process->read_bytes(&dynar, sizeof(dynar), remote_dynar);

  smx_actor_t* data = static_cast<smx_actor_t*>(::operator new(dynar.elmsize * dynar.used));
  process->read_bytes(data, dynar.elmsize * dynar.used, simgrid::mc::RemotePtr<void>(dynar.data));

  // Load each element of the vector from the MCed process:
  for (unsigned int i = 0; i < dynar.used; ++i) {

    simgrid::mc::ActorInformation info;
    info.address  = simgrid::mc::RemotePtr<simgrid::kernel::actor::ActorImpl>(data[i]);
    info.hostname = nullptr;
    process->read_bytes(&info.copy, sizeof(info.copy), remote(data[i]));
    target.push_back(std::move(info));
  }
  ::operator delete(data);
}
namespace simgrid {
namespace mc {

void RemoteClient::refresh_simix()
{
  if (this->cache_flags_ & RemoteClient::cache_simix_processes)
    return;

  // TODO, avoid to reload `&simix_global`, `simix_global`, `*simix_global`

  static_assert(std::is_same<
      std::unique_ptr<simgrid::simix::Global>,
      decltype(simix_global)
    >::value, "Unexpected type for simix_global");
  static_assert(sizeof(simix_global) == sizeof(simgrid::simix::Global*),
    "Bad size for simix_global");

  RemotePtr<simgrid::simix::Global> simix_global_p{this->read_variable<simgrid::simix::Global*>("simix_global")};

  // simix_global = REMOTE(*simix_global)
  Remote<simgrid::simix::Global> simix_global =
    this->read<simgrid::simix::Global>(simix_global_p);

  MC_process_refresh_simix_actor_dynar(this, this->smx_actors_infos, remote(simix_global.getBuffer()->actors_vector));
  MC_process_refresh_simix_actor_dynar(this, this->smx_dead_actors_infos,
                                       remote(simix_global.getBuffer()->dead_actors_vector));

  this->cache_flags_ |= RemoteClient::cache_simix_processes;
}

}
}

/** Get the issuer of a simcall (`req->issuer`)
 *
 *  In split-process mode, it does the black magic necessary to get an address
 *  of a (shallow) copy of the data structure the issuer SIMIX actor in the local
 *  address space.
 *
 *  @param process the MCed process
 *  @param req     the simcall (copied in the local process)
 */
smx_actor_t MC_smx_simcall_get_issuer(s_smx_simcall const* req)
{
  xbt_assert(mc_model_checker != nullptr);

  // This is the address of the smx_actor in the MCed process:
  auto address = simgrid::mc::remote(req->issuer);

  // Lookup by address:
  for (auto& actor : mc_model_checker->process().actors())
    if (actor.address == address)
      return actor.copy.getBuffer();
  for (auto& actor : mc_model_checker->process().dead_actors())
    if (actor.address == address)
      return actor.copy.getBuffer();

  xbt_die("Issuer not found");
}

const char* MC_smx_actor_get_host_name(smx_actor_t actor)
{
  if (mc_model_checker == nullptr)
    return actor->host_->get_cname();

  simgrid::mc::RemoteClient* process = &mc_model_checker->process();

  /* HACK, Horrible hack to find the offset of the id in the simgrid::s4u::Host.

     Offsetof is not supported for non-POD types but this should
     work in practice for the targets currently supported by the MC
     as long as we do not add funny features to the Host class
     (such as virtual base).

     We are using a (C++11) unrestricted union in order to avoid
     any construction/destruction of the simgrid::s4u::Host.
  */
  union fake_host {
    simgrid::s4u::Host host;
    fake_host() { /* Nothing to do*/}
    ~fake_host() { /* Nothing to do*/}
  };
  fake_host foo;
  const size_t offset = (char*)&foo.host.get_name() - (char*)&foo.host;

  // Read the simgrid::xbt::string in the MCed process:
  simgrid::mc::ActorInformation* info     = actor_info_cast(actor);
  auto remote_string_address              = remote((simgrid::xbt::string_data*)((char*)actor->host_ + offset));
  simgrid::xbt::string_data remote_string = process->read(remote_string_address);
  char hostname[remote_string.len];
  process->read_bytes(hostname, remote_string.len + 1, remote(remote_string.data));
  info->hostname = mc_model_checker->get_host_name(hostname).c_str();
  return info->hostname;
}

const char* MC_smx_actor_get_name(smx_actor_t actor)
{
  simgrid::mc::RemoteClient* process = &mc_model_checker->process();
  if (mc_model_checker == nullptr)
    return actor->get_cname();

  simgrid::mc::ActorInformation* info = actor_info_cast(actor);
  if (info->name.empty()) {
    simgrid::xbt::string_data string_data = simgrid::xbt::string::to_string_data(actor->name_);
    info->name = process->read_string(remote(string_data.data), string_data.len);
  }
  return info->name.c_str();
}

#if HAVE_SMPI
int MC_smpi_process_count()
{
  if (mc_model_checker == nullptr)
    return smpi_process_count();
  int res;
  mc_model_checker->process().read_variable("process_count",
    &res, sizeof(res));
  return res;
}
#endif

unsigned long MC_smx_get_maxpid()
{
  unsigned long maxpid;
  mc_model_checker->process().read_variable("simix_process_maxpid",
    &maxpid, sizeof(maxpid));
  return maxpid;
}
