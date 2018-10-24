/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SAMPI_H_
#define SAMPI_H_

#include <stdlib.h>
#include <smpi/smpi.h>

#define AMPI_CALL(type, name, args)                                                                                    \
  type A##name args __attribute__((weak));                                                                             \
  type AP##name args;

#ifndef HAVE_SMPI
#define malloc(nbytes) _sampi_malloc(nbytes)
#define calloc(n_elm,elm_size) _sampi_calloc(n_elm,elm_size)
#define realloc(ptr,nbytes) _sampi_realloc(ptr,nbytes)
#define free(ptr) _sampi_free(ptr)
#endif

SG_BEGIN_DECL()

XBT_PUBLIC void* _sampi_malloc(size_t size);
XBT_PUBLIC void* _sampi_calloc(size_t n_elm, size_t elm_size);
XBT_PUBLIC void* _sampi_realloc(void *ptr, size_t size);
XBT_PUBLIC void _sampi_free(void* ptr);

AMPI_CALL(XBT_PUBLIC int, MPI_Iteration_in, (MPI_Comm comm))
AMPI_CALL(XBT_PUBLIC int, MPI_Iteration_out, (MPI_Comm comm))
AMPI_CALL(XBT_PUBLIC void, MPI_Migrate, (MPI_Comm comm))

SG_END_DECL()

#endif
