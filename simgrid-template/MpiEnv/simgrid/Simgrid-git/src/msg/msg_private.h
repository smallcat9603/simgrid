/* Copyright (c) 2004-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef METASIMGRID_PRIVATE_H
#define METASIMGRID_PRIVATE_H

#include "msg/msg.h"
#include "simgrid/simix.h"
#include "surf/surf.h"
#include "xbt/fifo.h"
#include "xbt/dynar.h"
#include "xbt/swag.h"
#include "xbt/dict.h"
#include "xbt/config.h"
#include "instr/instr_private.h"
SG_BEGIN_DECL()

/**************** datatypes **********************************/

/********************************* Task **************************************/


#define MSG_BT(ptr, m)                              \
  do {xbt_ex_t *_xbt_ex_t = xbt_new0(xbt_ex_t, 1); \
  /* build the exception */                                             \
  _xbt_ex_t->msg      = (bprintf(m));                                 \
  _xbt_ex_t->category = (xbt_errcat_t)(0);                   \
  _xbt_ex_t->value    = (0);                                 \
  _xbt_ex_t->procname = (char*)xbt_procname();               \
  _xbt_ex_t->pid      = xbt_getpid();                        \
  _xbt_ex_t->file     = (char*)__FILE__;                     \
  _xbt_ex_t->line     = __LINE__;                            \
  _xbt_ex_t->func     = (char*)_XBT_FUNCTION;                \
  _xbt_ex_t->bt_strings = NULL;                              \
  xbt_backtrace_current(_xbt_ex_t); \
  ptr = _xbt_ex_t; } while(0)

typedef struct simdata_task {
  smx_synchro_t compute;         /* SIMIX modeling of computation */
  smx_synchro_t comm;            /* SIMIX modeling of communication */
  double message_size;          /* Data size */
  double computation_amount;    /* Computation size */
  msg_process_t sender;
  msg_process_t receiver;
  msg_host_t source;
  double priority;
  double bound; /* Capping for CPU resource */
  double rate;  /* Capping for network resource */

  /* CPU affinity database of this task */
  xbt_dict_t affinity_mask_db; /* smx_host_t host => unsigned long mask */

  void *isused;  /* Indicates whether the task is used in SIMIX currently */
  int host_nb;                  /* ==0 if sequential task; parallel task if not */
  /*******  Parallel Tasks Only !!!! *******/
  smx_host_t *host_list;
  double *comp_amount;
  double *comm_amount;
} s_simdata_task_t;

/********************************* File **************************************/
typedef struct simdata_file {
  smx_file_t smx_file;
} s_simdata_file_t;

/*************** Begin GPU ***************/
typedef struct simdata_gpu_task {
  double computation_amount;    /* Computation size */
  double dispatch_latency;
  double collect_latency;
  int isused;  /* Indicates whether the task is used in SIMIX currently */
} s_simdata_gpu_task_t;
/*************** End GPU ***************/

/******************************* Process *************************************/

typedef struct simdata_process {
  msg_host_t m_host;              /* the host on which the process is running */
  msg_host_t put_host;            /* used for debugging purposes */
#ifdef MSG_USE_DEPRECATED
  m_channel_t put_channel;      /* used for debugging purposes */
#endif
  smx_synchro_t waiting_action;
  msg_task_t waiting_task;
  char **argv;                  /* arguments table if any */
  int argc;                     /* arguments number if any */
  msg_error_t last_errno;       /* the last value returned by a MSG_function */

  void* data;                   /* user data */
} s_simdata_process_t, *simdata_process_t;

typedef struct process_arg {
  const char *name;
  xbt_main_func_t code;
  void *data;
  msg_host_t m_host;
  int argc;
  char **argv;
  double kill_time;
} s_process_arg_t, *process_arg_t;

typedef struct msg_comm {
  smx_synchro_t s_comm;          /* SIMIX communication object encapsulated (the same for both processes) */
  msg_task_t task_sent;           /* task sent (NULL for the receiver) */
  msg_task_t *task_received;      /* where the task will be received (NULL for the sender) */
  msg_error_t status;           /* status of the communication once finished */
} s_msg_comm_t;


/******************************* VM *************************************/
typedef struct dirty_page {
  double prev_clock;
  double prev_remaining;
  msg_task_t task;
} s_dirty_page, *dirty_page_t;

XBT_PUBLIC_DATA(const char*) MSG_vm_get_property_value(msg_vm_t vm, const char *name);
XBT_PUBLIC_DATA(xbt_dict_t) MSG_vm_get_properties(msg_vm_t vm);
XBT_PUBLIC_DATA(void) MSG_vm_set_property_value(msg_vm_t vm, const char *name, void *value, void_f_pvoid_t free_ctn);
XBT_PUBLIC_DATA(msg_vm_t) MSG_vm_get_by_name(const char *name);
XBT_PUBLIC_DATA(const char*) MSG_vm_get_name(msg_vm_t vm);

/************************** Global variables ********************************/
typedef struct MSG_Global {
  xbt_fifo_t host;
#ifdef MSG_USE_DEPRECATED
  int max_channel;
#endif
  int session;
  int debug_multiple_use;
  unsigned long int sent_msg;   /* Total amount of messages sent during the simulation */
  void (*task_copy_callback) (msg_task_t task, msg_process_t src, msg_process_t dst);
  void_f_pvoid_t process_data_cleanup;
} s_MSG_Global_t, *MSG_Global_t;

/*extern MSG_Global_t msg_global;*/
XBT_PUBLIC_DATA(MSG_Global_t) msg_global;


/*************************************************************/

#ifdef MSG_USE_DEPRECATED
#  define PROCESS_SET_ERRNO(val) \
  (((simdata_process_t) SIMIX_process_self_get_data(SIMIX_process_self()))->last_errno=val)
#  define PROCESS_GET_ERRNO() \
  (((simdata_process_t) SIMIX_process_self_get_data(SIMIX_process_self()))->last_errno)
#define MSG_RETURN(val) do {PROCESS_SET_ERRNO(val);return(val);} while(0)
/* #define CHECK_ERRNO()  ASSERT((PROCESS_GET_ERRNO()!=MSG_HOST_FAILURE),"Host failed, you cannot call this function.") */

#else
#  define MSG_RETURN(val) return(val)
#endif

msg_host_t __MSG_host_create(smx_host_t workstation);
msg_storage_t __MSG_storage_create(smx_storage_t storage);
void __MSG_host_destroy(msg_host_t host);
void __MSG_storage_destroy(msg_storage_priv_t host);
void __MSG_file_destroy(msg_file_priv_t host);

void MSG_process_cleanup_from_SIMIX(smx_process_t smx_proc);
void MSG_process_create_from_SIMIX(smx_process_t *process, const char *name,
                                   xbt_main_func_t code, void *data,
                                   const char *hostname, double kill_time,
                                   int argc, char **argv,
                                   xbt_dict_t properties, int auto_restart,
                                   smx_process_t parent_process);
void MSG_comm_copy_data_from_SIMIX(smx_synchro_t comm, void* buff, size_t buff_size);

void MSG_post_create_environment(void);

static inline void *msg_host_resource_priv(const void *host) {
  return xbt_lib_get_level((void *)host, MSG_HOST_LEVEL);
}

void MSG_host_add_task(msg_host_t host, msg_task_t task);
void MSG_host_del_task(msg_host_t host, msg_task_t task);

/********** Tracing **********/
/* declaration of instrumentation functions from msg_task_instr.c */
void TRACE_msg_set_task_category(msg_task_t task, const char *category);
void TRACE_msg_task_create(msg_task_t task);
void TRACE_msg_task_execute_start(msg_task_t task);
void TRACE_msg_task_execute_end(msg_task_t task);
void TRACE_msg_task_destroy(msg_task_t task);
void TRACE_msg_task_get_start(void);
void TRACE_msg_task_get_end(double start_time, msg_task_t task);
int TRACE_msg_task_put_start(msg_task_t task);    //returns TRUE if the task_put_end must be called
void TRACE_msg_task_put_end(void);

/* declaration of instrumentation functions from msg_process_instr.c */
char *instr_process_id (msg_process_t proc, char *str, int len);
char *instr_process_id_2 (const char *process_name, int process_pid, char *str, int len);
void TRACE_msg_process_change_host(msg_process_t process, msg_host_t old_host,
                                   msg_host_t new_host);
void TRACE_msg_process_create (const char *process_name, int process_pid, msg_host_t host);
void TRACE_msg_process_destroy (const char *process_name, int process_pid, msg_host_t host);
void TRACE_msg_process_kill(smx_process_exit_status_t status, msg_process_t process);
void TRACE_msg_process_suspend(msg_process_t process);
void TRACE_msg_process_resume(msg_process_t process);
void TRACE_msg_process_sleep_in(msg_process_t process);   //called from msg/gos.c
void TRACE_msg_process_sleep_out(msg_process_t process);
void TRACE_msg_process_end(msg_process_t process);

/* declaration of instrumentation functions from instr_msg_vm.c */
char *instr_vm_id(msg_vm_t vm, char *str, int len);
char *instr_vm_id_2(const char *vm_name, char *str, int len);
void TRACE_msg_vm_change_host(msg_vm_t vm, msg_host_t old_host,
                                   msg_host_t new_host);
void TRACE_msg_vm_start(msg_vm_t vm);
void TRACE_msg_vm_create(const char *vm_name, msg_host_t host);
void TRACE_msg_vm_kill(msg_vm_t process);
void TRACE_msg_vm_suspend(msg_vm_t vm);
void TRACE_msg_vm_resume(msg_vm_t vm);
void TRACE_msg_vm_save(msg_vm_t vm);
void TRACE_msg_vm_restore(msg_vm_t vm);
void TRACE_msg_vm_end(msg_vm_t vm);

SG_END_DECL()
#endif
