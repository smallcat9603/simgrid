/* Copyright (c) 2014-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/* MC interface: definitions that non-MC modules must see, but not the user */

#include <unistd.h> // pread, pwrite

#include "src/mc/mc_mmu.hpp"
#include "src/mc/mc_private.hpp"
#include "src/mc/sosp/PageStore.hpp"
#include "src/mc/sosp/mc_snapshot.hpp"

#include "src/mc/sosp/ChunkedData.hpp"
#include <xbt/mmalloc.h>

using simgrid::mc::remote;

/** @brief Restore a snapshot of a region
 *
 *  If possible, the restoration will be incremental
 *  (the modified pages will not be touched).
 *
 *  @param start_addr
 *  @param page_count       Number of pages of the region
 *  @param pagenos
 */
void mc_restore_page_snapshot_region(simgrid::mc::RemoteClient* process, void* start_addr,
                                     simgrid::mc::ChunkedData const& pages_copy)
{
  for (size_t i = 0; i != pages_copy.page_count(); ++i) {
    // Otherwise, copy the page:
    void* target_page       = (void*)simgrid::mc::mmu::join(i, (std::uintptr_t)start_addr);
    const void* source_page = pages_copy.page(i);
    process->write_bytes(source_page, xbt_pagesize, remote(target_page));
  }
}

// ***** High level API

void mc_region_restore_sparse(simgrid::mc::RemoteClient* process, mc_mem_region_t reg)
{
  xbt_assert(((reg->permanent_address().address()) & (xbt_pagesize - 1)) == 0, "Not at the beginning of a page");
  xbt_assert(simgrid::mc::mmu::chunkCount(reg->size()) == reg->page_data().page_count());
  mc_restore_page_snapshot_region(process, (void*)reg->permanent_address().address(), reg->page_data());
}
