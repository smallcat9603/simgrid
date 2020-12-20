/* Copyright (c) 2006-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/s4u/Comm.hpp"
#include "simgrid/s4u/Mailbox.hpp"
#include "src/kernel/activity/MailboxImpl.hpp"

#include <simgrid/mailbox.h>

XBT_LOG_EXTERNAL_CATEGORY(s4u);
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(s4u_channel, s4u, "S4U Communication Mailboxes");

namespace simgrid {
namespace s4u {

const xbt::string& Mailbox::get_name() const
{
  return pimpl_->get_name();
}

const char* Mailbox::get_cname() const
{
  return pimpl_->get_cname();
}

Mailbox* Mailbox::by_name(const std::string& name)
{
  kernel::activity::MailboxImpl* mbox = kernel::activity::MailboxImpl::by_name_or_null(name);
  if (mbox == nullptr) {
    mbox = kernel::actor::simcall([&name] { return kernel::activity::MailboxImpl::by_name_or_create(name); });
  }
  return &mbox->piface_;
}

bool Mailbox::empty() const
{
  return pimpl_->comm_queue_.empty();
}

bool Mailbox::listen() const
{
  return not this->empty() || (pimpl_->permanent_receiver_ && not pimpl_->done_comm_queue_.empty());
}

aid_t Mailbox::listen_from() const
{
  kernel::activity::CommImplPtr comm = front();
  if (comm && comm->src_actor_)
    return comm->src_actor_->get_pid();
  else
    return -1;
}

bool Mailbox::ready() const
{
  bool comm_ready = false;
  if (not pimpl_->comm_queue_.empty()) {
    comm_ready = pimpl_->comm_queue_.front()->state_ == kernel::activity::State::DONE;

  } else if (pimpl_->permanent_receiver_ && not pimpl_->done_comm_queue_.empty()) {
    comm_ready = pimpl_->done_comm_queue_.front()->state_ == kernel::activity::State::DONE;
  }
  return comm_ready;
}

kernel::activity::CommImplPtr Mailbox::front() const
{
  return pimpl_->comm_queue_.empty() ? nullptr : pimpl_->comm_queue_.front();
}

void Mailbox::set_receiver(ActorPtr actor)
{
  kernel::actor::simcall([this, actor]() { this->pimpl_->set_receiver(actor); });
}

/** @brief get the receiver (process associated to the mailbox) */
ActorPtr Mailbox::get_receiver() const
{
  if (pimpl_->permanent_receiver_ == nullptr)
    return ActorPtr();
  return pimpl_->permanent_receiver_->get_iface();
}

CommPtr Mailbox::put_init()
{
  CommPtr res(new Comm());
  res->sender_  = kernel::actor::ActorImpl::self();
  res->mailbox_ = this;
  return res;
}

CommPtr Mailbox::put_init(void* data, uint64_t simulated_size_in_bytes)
{
  CommPtr res = put_init();
  res->set_remaining(simulated_size_in_bytes);
  res->src_buff_      = data;
  res->src_buff_size_ = sizeof(void*);
  return res;
}

CommPtr Mailbox::put_async(void* payload, uint64_t simulated_size_in_bytes)
{
  xbt_assert(payload != nullptr, "You cannot send nullptr");

  CommPtr res = put_init(payload, simulated_size_in_bytes);
  res->vetoable_start();
  return res;
}
void Mailbox::put(void* payload, uint64_t simulated_size_in_bytes)
{
  xbt_assert(payload != nullptr, "You cannot send nullptr");

  CommPtr c = put_init();
  c->set_remaining(simulated_size_in_bytes);
  c->set_src_data(payload);
  c->vetoable_start();
  c->wait();
}
/** Blocking send with timeout */
void Mailbox::put(void* payload, uint64_t simulated_size_in_bytes, double timeout)
{
  xbt_assert(payload != nullptr, "You cannot send nullptr");

  CommPtr c = put_init();
  c->set_remaining(simulated_size_in_bytes);
  c->set_src_data(payload);
  c->vetoable_start();
  c->wait_for(timeout);
}

CommPtr Mailbox::get_init()
{
  CommPtr res(new Comm());
  res->receiver_ = kernel::actor::ActorImpl::self();
  res->mailbox_  = this;
  return res;
}

CommPtr Mailbox::get_async(void** data)
{
  CommPtr res = get_init();
  res->set_dst_data(data, sizeof(*data));
  res->vetoable_start();
  return res;
}

void* Mailbox::get()
{
  void* res = nullptr;
  CommPtr c = get_init();
  c->set_dst_data(&res, sizeof(res));
  c->vetoable_start();
  c->wait();
  return res;
}
void* Mailbox::get(double timeout)
{
  void* res = nullptr;
  CommPtr c = get_init();
  c->set_dst_data(&res, sizeof(res));
  c->vetoable_start();
  c->wait_for(timeout);
  return res;
}

kernel::activity::ActivityImplPtr
Mailbox::iprobe(int type, bool (*match_fun)(void*, void*, kernel::activity::CommImpl*), void* data)
{
  return kernel::actor::simcall([this, type, match_fun, data] { return pimpl_->iprobe(type, match_fun, data); });
}
} // namespace s4u
} // namespace simgrid

/* **************************** Public C interface *************************** */
sg_mailbox_t sg_mailbox_by_name(const char* alias)
{
  return simgrid::s4u::Mailbox::by_name(alias);
}

const char* sg_mailbox_get_name(const_sg_mailbox_t mailbox)
{
  return mailbox->get_cname();
}

/** @brief Set the mailbox to receive in asynchronous mode
 *
 * All messages sent to this mailbox will be transferred to the receiver without waiting for the receive call.
 * The receive call will still be necessary to use the received data.
 * If there is a need to receive some messages asynchronously, and some not, two different mailboxes should be used.
 *
 * @param alias The name of the mailbox
 */
void sg_mailbox_set_receiver(const char* alias)
{
  simgrid::s4u::Mailbox::by_name(alias)->set_receiver(simgrid::s4u::Actor::self());
  XBT_VERB("%s mailbox set to receive eagerly for myself\n", alias);
}

/** @brief Check if there is a communication going on in a mailbox.
 *
 * @param alias the name of the mailbox to be considered
 * @return Returns 1 if there is a communication, 0 otherwise
 */
int sg_mailbox_listen(const char* alias)
{
  return simgrid::s4u::Mailbox::by_name(alias)->listen() ? 1 : 0;
}

void* sg_mailbox_get(sg_mailbox_t mailbox)
{
  return mailbox->get();
}

sg_comm_t sg_mailbox_get_async(sg_mailbox_t mailbox, void** data)
{
  auto comm = mailbox->get_async(data);
  comm->add_ref();
  return comm.get();
}

void sg_mailbox_put(sg_mailbox_t mailbox, void* payload, long simulated_size_in_bytes)
{
  mailbox->put(payload, simulated_size_in_bytes);
}

sg_comm_t sg_mailbox_put_async(sg_mailbox_t mailbox, void* payload, long simulated_size_in_bytes)
{
  auto comm = mailbox->put_async(payload, simulated_size_in_bytes);
  comm->add_ref();
  return comm.get();
}

sg_comm_t sg_mailbox_put_init(sg_mailbox_t mailbox, void* payload, long simulated_size_in_bytes)
{
  auto comm = mailbox->put_init(payload, simulated_size_in_bytes);
  comm->add_ref();
  return comm.get();
}
