/* Copyright (c) 2006-2018. The SimGrid Team. All rights reserved.          */

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

const simgrid::xbt::string& Mailbox::get_name() const
{
  return pimpl_->get_name();
}

const char* Mailbox::get_cname() const
{
  return pimpl_->get_cname();
}

MailboxPtr Mailbox::by_name(std::string name)
{
  kernel::activity::MailboxImpl* mbox = kernel::activity::MailboxImpl::by_name_or_null(name);
  if (mbox == nullptr) {
    mbox = simix::simcall([name] { return kernel::activity::MailboxImpl::by_name_or_create(name); });
  }
  return MailboxPtr(&mbox->piface_, true);
}

bool Mailbox::empty()
{
  return pimpl_->comm_queue_.empty();
}

bool Mailbox::listen()
{
  return not this->empty() || (pimpl_->permanent_receiver_ && not pimpl_->done_comm_queue_.empty());
}

bool Mailbox::ready()
{
  bool comm_ready = false;
  if (not pimpl_->comm_queue_.empty()) {
    comm_ready = pimpl_->comm_queue_.front()->state_ == SIMIX_DONE;
    
  } else if (pimpl_->permanent_receiver_ && not pimpl_->done_comm_queue_.empty()) {
    comm_ready = pimpl_->done_comm_queue_.front()->state_ == SIMIX_DONE;
  }
  return comm_ready;
}

smx_activity_t Mailbox::front()
{
  return pimpl_->comm_queue_.empty() ? nullptr : pimpl_->comm_queue_.front();
}

void Mailbox::set_receiver(ActorPtr actor)
{
  simix::simcall([this, actor]() { this->pimpl_->set_receiver(actor); });
}

/** @brief get the receiver (process associated to the mailbox) */
ActorPtr Mailbox::get_receiver()
{
  if (pimpl_->permanent_receiver_ == nullptr)
    return ActorPtr();
  return pimpl_->permanent_receiver_->iface();
}

CommPtr Mailbox::put_init()
{
  CommPtr res   = CommPtr(new s4u::Comm());
  res->sender_  = SIMIX_process_self();
  res->mailbox_ = this;
  return res;
}
s4u::CommPtr Mailbox::put_init(void* data, uint64_t simulated_size_in_bytes)
{
  s4u::CommPtr res = put_init();
  res->set_remaining(simulated_size_in_bytes);
  res->src_buff_      = data;
  res->src_buff_size_ = sizeof(void*);
  return res;
}
s4u::CommPtr Mailbox::put_async(void* payload, uint64_t simulated_size_in_bytes)
{
  xbt_assert(payload != nullptr, "You cannot send nullptr");

  s4u::CommPtr res = put_init(payload, simulated_size_in_bytes);
  res->start();
  return res;
}
void Mailbox::put(void* payload, uint64_t simulated_size_in_bytes)
{
  xbt_assert(payload != nullptr, "You cannot send nullptr");

  CommPtr c = put_init();
  c->set_remaining(simulated_size_in_bytes);
  c->set_src_data(payload);
  c->wait();
}
/** Blocking send with timeout */
void Mailbox::put(void* payload, uint64_t simulated_size_in_bytes, double timeout)
{
  xbt_assert(payload != nullptr, "You cannot send nullptr");

  CommPtr c = put_init();
  c->set_remaining(simulated_size_in_bytes);
  c->set_src_data(payload);
  // c->start() is optional.
  c->wait_for(timeout);
}

s4u::CommPtr Mailbox::get_init()
{
  CommPtr res    = CommPtr(new s4u::Comm());
  res->receiver_ = SIMIX_process_self();
  res->mailbox_  = this;
  return res;
}
s4u::CommPtr Mailbox::get_async(void** data)
{
  s4u::CommPtr res = get_init();
  res->set_dst_data(data, sizeof(*data));
  res->start();
  return res;
}

void* Mailbox::get()
{
  void* res = nullptr;
  CommPtr c = get_init();
  c->set_dst_data(&res, sizeof(res));
  c->wait();
  return res;
}
void* Mailbox::get(double timeout)
{
  void* res = nullptr;
  CommPtr c = get_init();
  c->set_dst_data(&res, sizeof(res));
  c->wait_for(timeout);
  return res;
}
} // namespace s4u
} // namespace simgrid

/* **************************** Public C interface *************************** */
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
