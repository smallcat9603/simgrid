/* Copyright (c) 2010-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/* Redefine the classical malloc/free/realloc functions so that they fit well in the mmalloc framework */

#include "mmprivate.h"
#include "xbt_modinter.h"
#include "internal_config.h"
#include <math.h>

//#define MM_LEGACY_VERBOSE 1 /* define this to see which version of malloc gets used */

/* The mmalloc() package can use a single implicit malloc descriptor
   for mmalloc/mrealloc/mfree operations which do not supply an explicit
   descriptor.  This allows mmalloc() to provide
   backwards compatibility with the non-mmap'd version. */
xbt_mheap_t __mmalloc_default_mdp = NULL;


static xbt_mheap_t __mmalloc_current_heap = NULL;     /* The heap we are currently using. */

xbt_mheap_t mmalloc_get_current_heap(void)
{
  return __mmalloc_current_heap;
}

void mmalloc_set_current_heap(xbt_mheap_t new_heap)
{
  __mmalloc_current_heap = new_heap;
}


#ifdef MMALLOC_WANT_OVERRIDE_LEGACY
#if 0 && defined(HAVE_GNU_LD)

#undef _GNU_SOURCE
#define _GNU_SOURCE 1
#include <dlfcn.h>

static void * (*real_malloc) (size_t) = NULL;
static void * (*real_realloc) (void*,size_t) = NULL;
static void * (*real_free) (void*) = NULL;

static void mm_gnuld_legacy_init(void) { /* This function is called from mmalloc_preinit(); it works even if it's static because all mm is in mm.c */
  real_malloc = (void * (*) (size_t)) dlsym(RTLD_NEXT, "malloc");
  real_realloc = (void * (*) (void*,size_t)) dlsym(RTLD_NEXT, "realloc");
  real_free = (void * (*) (void*)) dlsym(RTLD_NEXT, "free");
  __mmalloc_current_heap = __mmalloc_default_mdp;
} 

/* Hello pimple!
 * DL needs some memory while resolving the malloc symbol, that is somehow problematic
 * To that extend, we have a little area here living in .BSS that we return if asked for memory before the malloc is resolved.
 */
static int allocated_junk = 0; /* keep track of many blocks of our little area was already given to someone */
#define JUNK_SIZE 8
#define MAX_JUNK_AREAS (64 * 1024 / JUNK_SIZE)
static char junkareas[MAX_JUNK_AREAS][JUNK_SIZE];

/* This version use mmalloc if there is a current heap, or the legacy implem if not */
static void *malloc_or_calloc(size_t n, int setzero) {
  xbt_mheap_t mdp = __mmalloc_current_heap;
  void *ret;
#ifdef MM_LEGACY_VERBOSE
  static int warned_raw = 0;
  static int warned_mmalloc = 0;
#endif

  if (mdp) {
    LOCK(mdp);
    ret = mmalloc(mdp, n);
    UNLOCK(mdp);
    // This was already done by mmalloc:
    if (mdp->options & XBT_MHEAP_OPTION_MEMSET) {
      setzero = 0;
    }
#ifdef MM_LEGACY_VERBOSE
    if (!warned_mmalloc) {
      fprintf(stderr,"Using mmalloc; enabling the model-checker in cmake may have a bad impact on your simulation performance\n");
      warned_mmalloc = 1;
    }
#endif
  } else if (!real_malloc) {
      size_t needed_areas = n / JUNK_SIZE;
      if(needed_areas * JUNK_SIZE != n) needed_areas++;
      if (allocated_junk+needed_areas>=MAX_JUNK_AREAS) {
        fprintf(stderr,
          "Panic: real malloc symbol not resolved yet, and I already gave my little private memory chunk away.\n");
        exit(1);
      } else {
        size_t i = allocated_junk;
        allocated_junk += needed_areas;
        ret = junkareas[i];
      }
    }
  else {
#ifdef MM_LEGACY_VERBOSE
    if (!warned_raw) {
      fprintf(stderr,"Using system malloc after interception; you seem to be currently model-checking\n");
      warned_raw = 1;
    }
#endif
    ret = real_malloc(n);
  }
  if (ret && setzero) {
    memset(ret, 0, n);
  }
  return ret;
}

void *malloc(size_t n)
{
  return malloc_or_calloc(n, 0);
}

void *calloc(size_t nmemb, size_t size)
{
  return malloc_or_calloc(nmemb*size, 1);
}

void *realloc(void *p, size_t s)
{
  xbt_mheap_t mdp = __mmalloc_current_heap;
  void *ret;

  if (mdp) {
    LOCK(mdp);
    ret = mrealloc(mdp, p, s);
    UNLOCK(mdp);
  } else {
    ret = real_realloc(p,s);
  }

  return ret;
}

void free(void *p)
{
  if (p==NULL)
    return;
  if (p<(void*)junkareas || p>=(void*)(junkareas[MAX_JUNK_AREAS]) ) {
    // main use case

    xbt_mheap_t mdp = __mmalloc_current_heap;

    if (mdp) {
      LOCK(mdp);
      mfree(mdp, p);
      UNLOCK(mdp);
    } else {
      real_free(p);
    }
  } else {
    // We are in the junkarea.
    // This area is used to allocate memory at initilization time.

    if(allocated_junk && p==junkareas[allocated_junk-1]) {
      // Last junkarea. We can reuse it.
      allocated_junk--;
    } else {
      // We currently cannot reuse freed junkareas in the general case.
    }
  }
}


#else /* NO GNU_LD */
void *malloc(size_t n)
{
  xbt_mheap_t mdp = __mmalloc_current_heap ?: (xbt_mheap_t) mmalloc_preinit();

  LOCK(mdp);
  void *ret = mmalloc(mdp, n);
  UNLOCK(mdp);


  return ret;
}

void *calloc(size_t nmemb, size_t size)
{
  xbt_mheap_t mdp = __mmalloc_current_heap ?: (xbt_mheap_t) mmalloc_preinit();

  LOCK(mdp);
  void *ret = mmalloc(mdp, nmemb*size);
  UNLOCK(mdp);

  // This was already done in the callee:
  if(!(mdp->options & XBT_MHEAP_OPTION_MEMSET)) {
    memset(ret, 0, nmemb * size);
  }

  return ret;
}

void *realloc(void *p, size_t s)
{
  void *ret = NULL;
  xbt_mheap_t mdp = __mmalloc_current_heap ?: (xbt_mheap_t) mmalloc_preinit();

  LOCK(mdp);
  ret = mrealloc(mdp, p, s);
  UNLOCK(mdp);

  return ret;
}

void free(void *p)
{
  if (p != NULL) {
    xbt_mheap_t mdp = __mmalloc_current_heap ?: (xbt_mheap_t) mmalloc_preinit();

    LOCK(mdp);
    mfree(mdp, p);
    UNLOCK(mdp);
  }
}
#endif /* NO GNU_LD */
#endif /* WANT_MALLOC_OVERRIDE */
