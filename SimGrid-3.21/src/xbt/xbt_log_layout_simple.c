/* layout_simple - a dumb log layout                                        */

/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt/sysdep.h"
#include "src/xbt/log_private.h"

#include "simgrid/host.h" /* sg_host_self_get_name */
#include "simgrid/msg.h"  /* MSG_get_clock */
#include <stdio.h>

extern const char *xbt_log_priority_names[8];
extern int xbt_log_no_loc;

#define check_overflow(len)                                             \
  if ((rem_size -= (len)) > 0) {                                        \
    p += (len);                                                         \
  } else                                                                \
    return 0

static int xbt_log_layout_simple_doit(XBT_ATTRIB_UNUSED xbt_log_layout_t l, xbt_log_event_t ev, const char* fmt)
{
  char *p = ev->buffer;
  int rem_size = ev->buffer_size;
  const char *procname;
  int len;

  *p = '[';
  check_overflow(1);

  /* Display the proc info if available */
  procname = xbt_procname();
  if (procname && strcmp(procname,"maestro")) {
    len = snprintf(p, rem_size, "%s:%s:(%d) ", sg_host_self_get_name(), procname, xbt_getpid());
    check_overflow(len);
  }
  else if (!procname)  {
    len = snprintf(p, rem_size, "%s::(%d) ", sg_host_self_get_name(), xbt_getpid());
    check_overflow(len);
  }

  /* Display the date */
  len = snprintf(p, rem_size, "%f] ", MSG_get_clock());
  check_overflow(len);

  /* Display file position if not INFO */
  if (ev->priority != xbt_log_priority_info && !xbt_log_no_loc) {
    len = snprintf(p, rem_size, "%s:%d: ", ev->fileName, ev->lineNum);
    check_overflow(len);
  }

  /* Display category name */
  len = snprintf(p, rem_size, "[%s/%s] ", ev->cat->name, xbt_log_priority_names[ev->priority]);
  check_overflow(len);

  /* Display user-provided message */
  len = vsnprintf(p, rem_size, fmt, ev->ap);
  check_overflow(len);

  /* End it */
  *p = '\n';
  check_overflow(1);
  *p = '\0';

  return 1;
}

xbt_log_layout_t xbt_log_layout_simple_new(XBT_ATTRIB_UNUSED char* arg)
{
  xbt_log_layout_t res = xbt_new0(s_xbt_log_layout_t, 1);
  res->do_layout       = &xbt_log_layout_simple_doit;

  return res;
}
