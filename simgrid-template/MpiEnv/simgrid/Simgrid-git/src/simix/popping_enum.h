/**********************************************************************/
/* File generated by src/simix/simcalls.py from src/simix/simcalls.in */
/*                                                                    */
/*                    DO NOT EVER CHANGE THIS FILE                    */
/*                                                                    */
/* change simcalls specification in src/simix/simcalls.in             */
/**********************************************************************/

/*
 * Note that the name comes from http://en.wikipedia.org/wiki/Popping
 * Indeed, the control flow is doing a strange dance in there.
 *
 * That's not about http://en.wikipedia.org/wiki/Poop, despite the odor :)
 */

/**
 * @brief All possible simcalls.
 */
typedef enum {
  SIMCALL_NONE,
  SIMCALL_HOST_GET_BY_NAME,
  SIMCALL_HOST_GET_NAME,
  SIMCALL_HOST_ON,
  SIMCALL_HOST_OFF,
  SIMCALL_HOST_GET_PROPERTIES,
  SIMCALL_HOST_GET_CORE,
  SIMCALL_HOST_GET_PROCESS_LIST,
  SIMCALL_HOST_GET_SPEED,
  SIMCALL_HOST_GET_AVAILABLE_SPEED,
  SIMCALL_HOST_GET_STATE,
  SIMCALL_HOST_GET_CURRENT_POWER_PEAK,
  SIMCALL_HOST_GET_POWER_PEAK_AT,
  SIMCALL_HOST_GET_NB_PSTATES,
  SIMCALL_HOST_SET_POWER_PEAK_AT,
  SIMCALL_HOST_GET_CONSUMED_ENERGY,
  SIMCALL_HOST_EXECUTE,
  SIMCALL_HOST_PARALLEL_EXECUTE,
  SIMCALL_HOST_EXECUTION_DESTROY,
  SIMCALL_HOST_EXECUTION_CANCEL,
  SIMCALL_HOST_EXECUTION_GET_REMAINS,
  SIMCALL_HOST_EXECUTION_GET_STATE,
  SIMCALL_HOST_EXECUTION_SET_PRIORITY,
  SIMCALL_HOST_EXECUTION_SET_BOUND,
  SIMCALL_HOST_EXECUTION_SET_AFFINITY,
  SIMCALL_HOST_EXECUTION_WAIT,
  SIMCALL_HOST_GET_MOUNTED_STORAGE_LIST,
  SIMCALL_HOST_GET_ATTACHED_STORAGE_LIST,
  SIMCALL_HOST_GET_PARAMS,
  SIMCALL_HOST_SET_PARAMS,
  SIMCALL_VM_CREATE,
  SIMCALL_VM_START,
  SIMCALL_VM_GET_STATE,
  SIMCALL_VM_MIGRATE,
  SIMCALL_VM_GET_PM,
  SIMCALL_VM_SET_BOUND,
  SIMCALL_VM_SET_AFFINITY,
  SIMCALL_VM_DESTROY,
  SIMCALL_VM_SUSPEND,
  SIMCALL_VM_RESUME,
  SIMCALL_VM_SHUTDOWN,
  SIMCALL_VM_SAVE,
  SIMCALL_VM_RESTORE,
  SIMCALL_PROCESS_CREATE,
  SIMCALL_PROCESS_KILL,
  SIMCALL_PROCESS_KILLALL,
  SIMCALL_PROCESS_CLEANUP,
  SIMCALL_PROCESS_CHANGE_HOST,
  SIMCALL_PROCESS_SUSPEND,
  SIMCALL_PROCESS_RESUME,
  SIMCALL_PROCESS_COUNT,
  SIMCALL_PROCESS_GET_PID,
  SIMCALL_PROCESS_GET_PPID,
  SIMCALL_PROCESS_GET_DATA,
  SIMCALL_PROCESS_SET_DATA,
  SIMCALL_PROCESS_GET_HOST,
  SIMCALL_PROCESS_GET_NAME,
  SIMCALL_PROCESS_IS_SUSPENDED,
  SIMCALL_PROCESS_GET_PROPERTIES,
  SIMCALL_PROCESS_JOIN,
  SIMCALL_PROCESS_SLEEP,
  SIMCALL_PROCESS_ON_EXIT,
  SIMCALL_PROCESS_AUTO_RESTART_SET,
  SIMCALL_PROCESS_RESTART,
  SIMCALL_RDV_CREATE,
  SIMCALL_RDV_DESTROY,
  SIMCALL_RDV_COMM_COUNT_BY_HOST,
  SIMCALL_RDV_GET_HEAD,
  SIMCALL_RDV_SET_RECEIVER,
  SIMCALL_RDV_GET_RECEIVER,
  SIMCALL_COMM_IPROBE,
  SIMCALL_COMM_SEND,
  SIMCALL_COMM_ISEND,
  SIMCALL_COMM_RECV,
  SIMCALL_COMM_IRECV,
  SIMCALL_COMM_CANCEL,
  SIMCALL_COMM_WAITANY,
  SIMCALL_COMM_WAIT,
  SIMCALL_COMM_TEST,
  SIMCALL_COMM_TESTANY,
  SIMCALL_COMM_GET_REMAINS,
  SIMCALL_COMM_GET_STATE,
  SIMCALL_COMM_GET_SRC_DATA,
  SIMCALL_COMM_GET_DST_DATA,
  SIMCALL_COMM_GET_SRC_PROC,
  SIMCALL_COMM_GET_DST_PROC,
  SIMCALL_MUTEX_INIT,
  SIMCALL_MUTEX_DESTROY,
  SIMCALL_MUTEX_LOCK,
  SIMCALL_MUTEX_TRYLOCK,
  SIMCALL_MUTEX_UNLOCK,
  SIMCALL_COND_INIT,
  SIMCALL_COND_DESTROY,
  SIMCALL_COND_SIGNAL,
  SIMCALL_COND_WAIT,
  SIMCALL_COND_WAIT_TIMEOUT,
  SIMCALL_COND_BROADCAST,
  SIMCALL_SEM_INIT,
  SIMCALL_SEM_DESTROY,
  SIMCALL_SEM_RELEASE,
  SIMCALL_SEM_WOULD_BLOCK,
  SIMCALL_SEM_ACQUIRE,
  SIMCALL_SEM_ACQUIRE_TIMEOUT,
  SIMCALL_SEM_GET_CAPACITY,
  SIMCALL_FILE_READ,
  SIMCALL_FILE_WRITE,
  SIMCALL_FILE_OPEN,
  SIMCALL_FILE_CLOSE,
  SIMCALL_FILE_UNLINK,
  SIMCALL_FILE_GET_SIZE,
  SIMCALL_FILE_TELL,
  SIMCALL_FILE_SEEK,
  SIMCALL_FILE_GET_INFO,
  SIMCALL_FILE_MOVE,
  SIMCALL_STORAGE_GET_FREE_SIZE,
  SIMCALL_STORAGE_GET_USED_SIZE,
  SIMCALL_STORAGE_GET_PROPERTIES,
  SIMCALL_STORAGE_GET_CONTENT,
  SIMCALL_ASR_GET_PROPERTIES,
#ifdef HAVE_LATENCY_BOUND_TRACKING
  SIMCALL_COMM_IS_LATENCY_BOUNDED,
#endif

#ifdef HAVE_TRACING
  SIMCALL_SET_CATEGORY,
#endif

#ifdef HAVE_MC
  SIMCALL_MC_SNAPSHOT,
  SIMCALL_MC_COMPARE_SNAPSHOTS,
  SIMCALL_MC_RANDOM,
#endif
  NUM_SIMCALLS
} e_smx_simcall_t;
