#include "mc_page_store.h"
#include "mc_mmu.h"
#include "mc_private.h"

#include <xbt/mmalloc.h>

#define SOFT_DIRTY_BIT_NUMBER 55
#define SOFT_DIRTY (((uint64_t)1) << SOFT_DIRTY_BIT_NUMBER)

extern "C" {

// ***** Region management:

/** @brief Take a per-page snapshot of a region
 *
 *  @param data            The start of the region (must be at the beginning of a page)
 *  @param pag_count       Number of pages of the region
 *  @param pagemap         Linux kernel pagemap values fot this region (or NULL)
 *  @param reference_pages Snapshot page numbers of the previous soft_dirty_reset (or NULL)
 *  @return                Snapshot page numbers of this new snapshot
 */
size_t* mc_take_page_snapshot_region(void* data, size_t page_count, uint64_t* pagemap, size_t* reference_pages)
{
  size_t* pagenos = (size_t*) malloc(page_count * sizeof(size_t));

  for (size_t i=0; i!=page_count; ++i) {
    bool softclean = pagemap && !(pagemap[i] & SOFT_DIRTY);
    if (softclean && reference_pages) {
      // The page is softclean, it is the same page as the reference page:
      pagenos[i] = reference_pages[i];
      mc_model_checker->pages->ref_page(reference_pages[i]);
    } else {
      // Otherwise, we need to store the page the hard way
      // (by reading its content):
      void* page = (char*) data + (i << xbt_pagebits);
      pagenos[i] = mc_model_checker->pages->store_page(page);
    }
  }

  return pagenos;
}

void mc_free_page_snapshot_region(size_t* pagenos, size_t page_count)
{
  for (size_t i=0; i!=page_count; ++i) {
    mc_model_checker->pages->unref_page(pagenos[i]);
  }
}

/** @brief Restore a snapshot of a region
 *
 *  If possible, the restoration will be incremental
 *  (the modified pages will not be touched).
 *
 *  @param start_addr
 *  @param page_count       Number of pages of the region
 *  @param pagenos
 *  @param pagemap         Linux kernel pagemap values fot this region (or NULL)
 *  @param reference_pages Snapshot page numbers of the previous soft_dirty_reset (or NULL)
 */
void mc_restore_page_snapshot_region(void* start_addr, size_t page_count, size_t* pagenos, uint64_t* pagemap, size_t* reference_pagenos)
{
  for (size_t i=0; i!=page_count; ++i) {

    bool softclean = pagemap && !(pagemap[i] & SOFT_DIRTY);
    if (softclean && reference_pagenos && pagenos[i] == reference_pagenos[i]) {
      // The page is softclean and is the same as the reference one:
      // the page is already in the target state.
      continue;
    }

    // Otherwise, copy the page:
    void* target_page = mc_page_from_number(start_addr, i);
    const void* source_page = mc_model_checker->pages->get_page(pagenos[i]);
    memcpy(target_page, source_page, xbt_pagesize);
  }
}

// ***** Soft dirty tracking

/** @brief Like pread() but without partial reads */
static size_t pread_whole(int fd, void* buf, size_t count, off_t offset) {
  size_t res = 0;

  char* data = (char*) buf;
  while(count) {
    ssize_t n = pread(fd, buf, count, offset);
    // EOF
    if (n==0)
      return res;

    // Error (or EINTR):
    if (n==-1) {
      if (errno == EINTR)
        continue;
      else
        return -1;
    }

    // It might be a partial read:
    count -= n;
    data += n;
    offset += n;
    res += n;
  }

  return res;
}

static inline  __attribute__ ((always_inline))
void mc_ensure_fd(int* fd, const char* path, int flags) {
  if (*fd != -1)
    return;
  *fd = open(path, flags);
  if (*fd == -1) {
    xbt_die("Could not open file %s", path);
  }
}

/** @brief Reset the soft-dirty bits
 *
 *  This is done after checkpointing and after checkpoint restoration
 *  (if per page checkpoiting is used) in order to know which pages were
 *  modified.
 *
 *  See https://www.kernel.org/doc/Documentation/vm/soft-dirty.txt
 * */
void mc_softdirty_reset() {
  mc_ensure_fd(&mc_model_checker->fd_clear_refs, "/proc/self/clear_refs", O_WRONLY|O_CLOEXEC);
  if( ::write(mc_model_checker->fd_clear_refs, "4\n", 2) != 2) {
    xbt_die("Could not reset softdirty bits");
  }
}

/** @brief Read memory page informations
 *
 *  For each virtual memory page of the process,
 *  /proc/self/pagemap provides a 64 bit field of information.
 *  We are interested in the soft-dirty bit: with this we can track which
 *  pages were modified between snapshots/restorations and avoid
 *  copying data which was not modified.
 *
 *  See https://www.kernel.org/doc/Documentation/vm/pagemap.txt
 *
 *  @param pagemap    Output buffer for pagemap informations
 *  @param start_addr Address of the first page
 *  @param page_count Number of pages
 */
static void mc_read_pagemap(uint64_t* pagemap, size_t page_start, size_t page_count)
{
  mc_ensure_fd(&mc_model_checker->fd_pagemap, "/proc/self/pagemap", O_RDONLY|O_CLOEXEC);
  size_t bytesize = sizeof(uint64_t) * page_count;
  off_t offset = sizeof(uint64_t) * page_start;
  if (pread_whole(mc_model_checker->fd_pagemap, pagemap, bytesize, offset) != bytesize) {
    xbt_die("Could not read pagemap");
  }
}

// ***** High level API

mc_mem_region_t mc_region_new_sparse(int type, void *start_addr, void* permanent_addr, size_t size, mc_mem_region_t ref_reg)
{
  mc_mem_region_t new_reg = xbt_new(s_mc_mem_region_t, 1);

  new_reg->start_addr = start_addr;
  new_reg->permanent_addr = permanent_addr;
  new_reg->data = NULL;
  new_reg->size = size;
  new_reg->page_numbers = NULL;

  xbt_assert((((uintptr_t)start_addr) & (xbt_pagesize-1)) == 0,
    "Not at the beginning of a page");
  xbt_assert((((uintptr_t)permanent_addr) & (xbt_pagesize-1)) == 0,
    "Not at the beginning of a page");
  size_t page_count = mc_page_count(size);

  uint64_t* pagemap = NULL;
  if (_sg_mc_soft_dirty && mc_model_checker->parent_snapshot) {
      pagemap = (uint64_t*) mmalloc_no_memset(mc_heap, sizeof(uint64_t) * page_count);
      mc_read_pagemap(pagemap, mc_page_number(NULL, permanent_addr), page_count);
  }

  // Take incremental snapshot:
  new_reg->page_numbers = mc_take_page_snapshot_region(permanent_addr, page_count, pagemap,
    ref_reg==NULL ? NULL : ref_reg->page_numbers);

  if(pagemap) {
    mfree(mc_heap, pagemap);
  }
  return new_reg;
}

void mc_region_restore_sparse(mc_mem_region_t reg, mc_mem_region_t ref_reg)
{
  xbt_assert((((uintptr_t)reg->permanent_addr) & (xbt_pagesize-1)) == 0,
    "Not at the beginning of a page");
  size_t page_count = mc_page_count(reg->size);

  uint64_t* pagemap = NULL;

  // Read soft-dirty bits if necessary in order to know which pages have changed:
  if (_sg_mc_soft_dirty && mc_model_checker->parent_snapshot) {
    pagemap = (uint64_t*) mmalloc_no_memset(mc_heap, sizeof(uint64_t) * page_count);
    mc_read_pagemap(pagemap, mc_page_number(NULL, reg->permanent_addr), page_count);
  }

  // Incremental per-page snapshot restoration:
  mc_restore_page_snapshot_region(reg->permanent_addr, page_count, reg->page_numbers,
    pagemap, ref_reg ? ref_reg->page_numbers : NULL);

  if(pagemap) {
    free(pagemap);
  }
}

}
