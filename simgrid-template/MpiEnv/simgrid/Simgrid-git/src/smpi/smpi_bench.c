/* Copyright (c) 2007, 2009-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "internal_config.h"
#include "private.h"
#include "xbt/dict.h"
#include "xbt/sysdep.h"
#include "xbt/ex.h"
#include "xbt/hash.h"
#include "surf/surf.h"
#include "simgrid/sg_config.h"
#include "simgrid/modelchecker.h"

#ifndef WIN32
#include <sys/mman.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h> // sqrt
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(smpi_bench, smpi,
                                "Logging specific to SMPI (benchmarking)");

/* Shared allocations are handled through shared memory segments.
 * Associated data and metadata are used as follows:
 *
 *                                                                    mmap #1
 *    `allocs' dict                                                     ---- -.
 *    ----------      shared_data_t               shared_metadata_t   / |  |  |
 * .->| <name> | ---> -------------------- <--.   -----------------   | |  |  |
 * |  ----------      | fd of <name>     |    |   | size of mmap  | --| |  |  |
 * |                  | count (2)        |    |-- | data          |   \ |  |  |
 * `----------------- | <name>           |    |   -----------------     ----  |
 *                    --------------------    |   ^                           |
 *                                            |   |                           |
 *                                            |   |   `allocs_metadata' dict  |
 *                                            |   |   ----------------------  |
 *                                            |   `-- | <addr of mmap #1>  |<-'
 *                                            |   .-- | <addr of mmap #2>  |<-.
 *                                            |   |   ----------------------  |
 *                                            |   |                           |
 *                                            |   |                           |
 *                                            |   |                           |
 *                                            |   |                   mmap #2 |
 *                                            |   v                     ---- -'
 *                                            |   shared_metadata_t   / |  |
 *                                            |   -----------------   | |  |
 *                                            |   | size of mmap  | --| |  |
 *                                            `-- | data          |   | |  |
 *                                                -----------------   | |  |
 *                                                                    \ |  |
 *                                                                      ----
 */

#define PTR_STRLEN (2 + 2 * sizeof(void*) + 1)

xbt_dict_t allocs = NULL;          /* Allocated on first use */
xbt_dict_t allocs_metadata = NULL; /* Allocated on first use */
xbt_dict_t samples = NULL;         /* Allocated on first use */
xbt_dict_t calls = NULL;           /* Allocated on first use */

double smpi_cpu_threshold;
double smpi_running_power;

int smpi_loaded_page = -1;
char* start_data_exe = NULL;
int size_data_exe = 0;
int smpi_privatize_global_variables;
double smpi_total_benched_time = 0;


smpi_privatisation_region_t smpi_privatisation_regions;

typedef struct {
  int fd;
  int count;
  char* loc;
} shared_data_t;

typedef struct  {
  size_t size;
  shared_data_t* data;
} shared_metadata_t;

static size_t shm_size(int fd) {
  struct stat st;

  if(fstat(fd, &st) < 0) {
    xbt_die("Could not stat fd %d: %s", fd, strerror(errno));
  }
  return (size_t)st.st_size;
}

#ifndef WIN32
static void* shm_map(int fd, size_t size, shared_data_t* data) {
  void* mem;
  char loc[PTR_STRLEN];
  shared_metadata_t* meta;

  if(size > shm_size(fd)) {
    if(ftruncate(fd, (off_t)size) < 0) {
      xbt_die("Could not truncate fd %d to %zu: %s", fd, size, strerror(errno));
    }
  }

  mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(mem == MAP_FAILED) {
    xbt_die("Could not map fd %d: %s", fd, strerror(errno));
  }
  if(!allocs_metadata) {
    allocs_metadata = xbt_dict_new_homogeneous(xbt_free_f);
  }
  snprintf(loc, PTR_STRLEN, "%p", mem);
  meta = xbt_new(shared_metadata_t, 1);
  meta->size = size;
  meta->data = data;
  xbt_dict_set(allocs_metadata, loc, meta, NULL);
  XBT_DEBUG("MMAP %zu to %p", size, mem);
  return mem;
}
#endif

void smpi_bench_destroy(void)
{
  xbt_dict_free(&allocs);
  xbt_dict_free(&allocs_metadata);
  xbt_dict_free(&samples);
  xbt_dict_free(&calls);
}

XBT_PUBLIC(void) smpi_execute_flops_(double *flops);
void smpi_execute_flops_(double *flops)
{
  smpi_execute_flops(*flops);
}

XBT_PUBLIC(void) smpi_execute_(double *duration);
void smpi_execute_(double *duration)
{
  smpi_execute(*duration);
}

void smpi_execute_flops(double flops) {
  smx_synchro_t action;
  smx_host_t host;
  host = SIMIX_host_self();
  XBT_DEBUG("Handle real computation time: %f flops", flops);
  action = simcall_host_execute("computation", host, flops, 1, 0, 0);
#ifdef HAVE_TRACING
  simcall_set_category (action, TRACE_internal_smpi_get_category());
#endif
  simcall_host_execution_wait(action);
  smpi_switch_data_segment(smpi_process_index());
}

void smpi_execute(double duration)
{
  if (duration >= smpi_cpu_threshold) {
    XBT_DEBUG("Sleep for %g to handle real computation time", duration);
    double flops = duration * smpi_running_power;
#ifdef HAVE_TRACING
    int rank = smpi_process_index();
    instr_extra_data extra = xbt_new0(s_instr_extra_data_t,1);
    extra->type=TRACING_COMPUTING;
    extra->comp_size=flops;
    TRACE_smpi_computing_in(rank, extra);
#endif
    smpi_execute_flops(flops);

#ifdef HAVE_TRACING
    TRACE_smpi_computing_out(rank);
#endif

  } else {
    XBT_DEBUG("Real computation took %g while option smpi/cpu_threshold is set to %g => ignore it",
              duration, smpi_cpu_threshold);
  }
}

void smpi_switch_data_segment(int dest);

void smpi_bench_begin(void)
{
  smpi_switch_data_segment(smpi_process_index());

  if(MC_is_active())
    return;

  xbt_os_threadtimer_start(smpi_process_timer());
}

void smpi_bench_end(void)
{

  if(MC_is_active())
    return;

  xbt_os_timer_t timer = smpi_process_timer();
  xbt_os_threadtimer_stop(timer);
//  smpi_switch_data_segment(smpi_process_count());
  if (smpi_process_get_sampling()) {
    XBT_CRITICAL("Cannot do recursive benchmarks.");
    XBT_CRITICAL("Are you trying to make a call to MPI within a SMPI_SAMPLE_ block?");
    xbt_backtrace_display_current();
    xbt_die("Aborting.");
  }
  // Simulate the benchmarked computation unless disabled via command-line argument
  if (sg_cfg_get_boolean("smpi/simulate_computation")) {
    smpi_execute(xbt_os_timer_elapsed(timer));
  }

  smpi_total_benched_time += xbt_os_timer_elapsed(timer);
}

/* Private sleep function used by smpi_sleep() and smpi_usleep() */
static unsigned int private_sleep(double secs)
{
  smpi_bench_end();

  XBT_DEBUG("Sleep for: %lf secs", secs);
  #ifdef HAVE_TRACING
  int rank = smpi_comm_rank(MPI_COMM_WORLD);
  instr_extra_data extra = xbt_new0(s_instr_extra_data_t,1);
  extra->type=TRACING_SLEEPING;
  extra->sleep_duration=secs;
  TRACE_smpi_sleeping_in(rank, extra);
#endif
  simcall_process_sleep(secs);
#ifdef HAVE_TRACING
  TRACE_smpi_sleeping_out(rank);
#endif

  smpi_bench_begin();
  return 0;
}

unsigned int smpi_sleep(unsigned int secs)
{
  return private_sleep((double)secs);
}

int smpi_usleep(useconds_t usecs)
{
  return (int)private_sleep((double)usecs / 1000000.0);
}


int smpi_gettimeofday(struct timeval *tv, void* tz)
{
  double now;
  smpi_bench_end();
  now = SIMIX_get_clock();
  if (tv) {
    tv->tv_sec = (time_t)now;
#ifdef WIN32
    tv->tv_usec = (useconds_t)((now - tv->tv_sec) * 1e6);
#else
    tv->tv_usec = (suseconds_t)((now - tv->tv_sec) * 1e6);
#endif
  }
  smpi_bench_begin();
  return 0;
}

extern double sg_surf_precision;
unsigned long long smpi_rastro_resolution (void)
{
  smpi_bench_end();
  double resolution = (1/sg_surf_precision);
  smpi_bench_begin();
  return (unsigned long long)resolution;
}

unsigned long long smpi_rastro_timestamp (void)
{
  smpi_bench_end();
  double now = SIMIX_get_clock();

  unsigned long long sec = (unsigned long long)now;
  unsigned long long pre = (now - sec) * smpi_rastro_resolution();
  smpi_bench_begin();
  return (unsigned long long)sec * smpi_rastro_resolution() + pre;
}

/* ****************************** Functions related to the SMPI_SAMPLE_ macros ************************************/
typedef struct {
  double threshold; /* maximal stderr requested (if positive) */
  double relstderr; /* observed stderr so far */
  double mean;      /* mean of benched times, to be used if the block is disabled */
  double sum;       /* sum of benched times (to compute the mean and stderr) */
  double sum_pow2;  /* sum of the square of the benched times (to compute the stderr) */
  int iters;        /* amount of requested iterations */
  int count;        /* amount of iterations done so far */
  int benching;     /* 1: we are benchmarking; 0: we have enough data, no bench anymore */
} local_data_t;

static char *sample_location(int global, const char *file, int line) {
  if (global) {
    return bprintf("%s:%d", file, line);
  } else {
    return bprintf("%s:%d:%d", file, line, smpi_process_index());
  }
}
static int sample_enough_benchs(local_data_t *data) {
  int res = data->count >= data->iters;
  if (data->threshold>0.0) {
    if (data->count <2)
      res = 0; // not enough data
    if (data->relstderr > data->threshold)
      res = 0; // stderr too high yet
  }
  XBT_DEBUG("%s (count:%d iter:%d stderr:%f thres:%f mean:%fs)",
      (res?"enough benchs":"need more data"),
      data->count, data->iters, data->relstderr, data->threshold, data->mean);
  return res;
}

void smpi_sample_1(int global, const char *file, int line, int iters, double threshold)
{
  char *loc = sample_location(global, file, line);
  local_data_t *data;

  smpi_bench_end();     /* Take time from previous, unrelated computation into account */
  smpi_process_set_sampling(1);

  if (!samples)
    samples = xbt_dict_new_homogeneous(free);

  data = xbt_dict_get_or_null(samples, loc);
  if (!data) {
    xbt_assert(threshold>0 || iters>0,
        "You should provide either a positive amount of iterations to bench, or a positive maximal stderr (or both)");
    data = (local_data_t *) xbt_new(local_data_t, 1);
    data->count = 0;
    data->sum = 0.0;
    data->sum_pow2 = 0.0;
    data->iters = iters;
    data->threshold = threshold;
    data->benching = 1; // If we have no data, we need at least one
    data->mean = 0;
    xbt_dict_set(samples, loc, data, NULL);
    XBT_DEBUG("XXXXX First time ever on benched nest %s.",loc);
  } else {
    if (data->iters != iters || data->threshold != threshold) {
      XBT_ERROR("Asked to bench block %s with different settings %d, %f is not %d, %f. How did you manage to give two numbers at the same line??",
          loc, data->iters, data->threshold, iters,threshold);
      THROW_IMPOSSIBLE;
    }

    // if we already have some data, check whether sample_2 should get one more bench or whether it should emulate the computation instead
    data->benching = !sample_enough_benchs(data);
    XBT_DEBUG("XXXX Re-entering the benched nest %s. %s",loc, (data->benching?"more benching needed":"we have enough data, skip computes"));
  }
  xbt_free(loc);
}

int smpi_sample_2(int global, const char *file, int line)
{
  char *loc = sample_location(global, file, line);
  local_data_t *data;
  int res;

  xbt_assert(samples, "Y U NO use SMPI_SAMPLE_* macros? Stop messing directly with smpi_sample_* functions!");
  data = xbt_dict_get(samples, loc);
  XBT_DEBUG("sample2 %s",loc);
  xbt_free(loc);

  if (data->benching==1) {
    // we need to run a new bench
    XBT_DEBUG("benchmarking: count:%d iter:%d stderr:%f thres:%f; mean:%f",
        data->count, data->iters, data->relstderr, data->threshold, data->mean);
    res = 1;
  } else {
    // Enough data, no more bench (either we got enough data from previous visits to this benched nest, or we just ran one bench and need to bail out now that our job is done).
    // Just sleep instead
    XBT_DEBUG("No benchmark (either no need, or just ran one): count >= iter (%d >= %d) or stderr<thres (%f<=%f). apply the %fs delay instead",
        data->count, data->iters, data->relstderr, data->threshold, data->mean);
    smpi_execute(data->mean);
    smpi_process_set_sampling(0);
    res = 0; // prepare to capture future, unrelated computations
  }
  smpi_bench_begin();
  return res;
}


void smpi_sample_3(int global, const char *file, int line)
{
  char *loc = sample_location(global, file, line);
  local_data_t *data;

  xbt_assert(samples, "Y U NO use SMPI_SAMPLE_* macros? Stop messing directly with smpi_sample_* functions!");
  data = xbt_dict_get(samples, loc);
  XBT_DEBUG("sample3 %s",loc);
  xbt_free(loc);

  if (data->benching==0) {
    THROW_IMPOSSIBLE;
  }

  // ok, benchmarking this loop is over
  xbt_os_threadtimer_stop(smpi_process_timer());

  // update the stats
  double sample, n;
  data->count++;
  sample = xbt_os_timer_elapsed(smpi_process_timer());
  data->sum += sample;
  data->sum_pow2 += sample * sample;
  n = (double)data->count;
  data->mean = data->sum / n;
  data->relstderr = sqrt((data->sum_pow2 / n - data->mean * data->mean) / n) / data->mean;
  if (!sample_enough_benchs(data)) {
    data->mean = sample; // Still in benching process; We want sample_2 to simulate the exact time of this loop occurrence before leaving, not the mean over the history
  }
  XBT_DEBUG("Average mean after %d steps is %f, relative standard error is %f (sample was %f)", data->count,
      data->mean, data->relstderr, sample);

  // That's enough for now, prevent sample_2 to run the same code over and over
  data->benching = 0;
}

#ifndef WIN32
static void smpi_shared_alloc_free(void *p)
{
  shared_data_t *data = p;
  xbt_free(data->loc);
  xbt_free(data);
}

static char *smpi_shared_alloc_hash(char *loc)
{
  char hash[42];
  char s[7];
  unsigned val;
  int i, j;

  xbt_sha(loc, hash);
  hash[41] = '\0';
  s[6] = '\0';
  loc = xbt_realloc(loc, 30);
  loc[0] = '/';
  for (i = 0; i < 40; i += 6) { /* base64 encode */
    memcpy(s, hash + i, 6);
    val = strtoul(s, NULL, 16);
    for (j = 0; j < 4; j++) {
      unsigned char x = (val >> (18 - 3 * j)) & 0x3f;
      loc[1 + 4 * i / 6 + j] =
        "ABCDEFGHIJKLMNOPQRSTUVZXYZabcdefghijklmnopqrstuvzxyz0123456789-_"[x];
    }
  }
  loc[29] = '\0';
  return loc;
}

void *smpi_shared_malloc(size_t size, const char *file, int line)
{
  void* mem;
  if (sg_cfg_get_boolean("smpi/use_shared_malloc")){
    char *loc = bprintf("%zu_%s_%d", (size_t)getpid(), file, line);
    int fd;
    shared_data_t *data;
    loc = smpi_shared_alloc_hash(loc); /* hash loc, in order to have something
                                        * not too long */
    if (!allocs) {
      allocs = xbt_dict_new_homogeneous(smpi_shared_alloc_free);
    }
    data = xbt_dict_get_or_null(allocs, loc);
    if (!data) {
      fd = shm_open(loc, O_RDWR | O_CREAT | O_EXCL,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
      if (fd < 0) {
        switch(errno) {
          case EEXIST:
            xbt_die("Please cleanup /dev/shm/%s", loc);
          default:
            xbt_die("An unhandled error occured while opening %s. shm_open: %s", loc, strerror(errno));
        }
      }
      data = xbt_new(shared_data_t, 1);
      data->fd = fd;
      data->count = 1;
      data->loc = loc;
      mem = shm_map(fd, size, data);
      if (shm_unlink(loc) < 0) {
        XBT_WARN("Could not early unlink %s. shm_unlink: %s", loc, strerror(errno));
      }
      xbt_dict_set(allocs, loc, data, NULL);
      XBT_DEBUG("Mapping %s at %p through %d", loc, mem, fd);
    } else {
      xbt_free(loc);
      mem = shm_map(data->fd, size, data);
      data->count++;
    }
    XBT_DEBUG("Shared malloc %zu in %p (metadata at %p)", size, mem, data);
  } else {
    mem = xbt_malloc(size);
    XBT_DEBUG("Classic malloc %zu in %p", size, mem);
  }

  return mem;
}
void smpi_shared_free(void *ptr)
{
  char loc[PTR_STRLEN];
  shared_metadata_t* meta;
  shared_data_t* data;
  if (sg_cfg_get_boolean("smpi/use_shared_malloc")){

    if (!allocs) {
      XBT_WARN("Cannot free: nothing was allocated");
      return;
    }
    if(!allocs_metadata) {
      XBT_WARN("Cannot free: no metadata was allocated");
    }
    snprintf(loc, PTR_STRLEN, "%p", ptr);
    meta = (shared_metadata_t*)xbt_dict_get_or_null(allocs_metadata, loc);
    if (!meta) {
      XBT_WARN("Cannot free: %p was not shared-allocated by SMPI", ptr);
      return;
    }
    data = meta->data;
    if(!data) {
      XBT_WARN("Cannot free: something is broken in the metadata link");
      return;
    }
    if(munmap(ptr, meta->size) < 0) {
      XBT_WARN("Unmapping of fd %d failed: %s", data->fd, strerror(errno));
    }
    data->count--;
    XBT_DEBUG("Shared free - no removal - of %p, count = %d", ptr, data->count);
    if (data->count <= 0) {
      close(data->fd);
      xbt_dict_remove(allocs, data->loc);
      XBT_DEBUG("Shared free - with removal - of %p", ptr);
    }
  }else{
    XBT_DEBUG("Classic free of %p", ptr);
    xbt_free(ptr);
  }
}
#endif

int smpi_shared_known_call(const char* func, const char* input)
{
  char* loc = bprintf("%s:%s", func, input);
  xbt_ex_t ex;
  int known = 0;

  if (!calls) {
    calls = xbt_dict_new_homogeneous(NULL);
  }
  TRY {
    xbt_dict_get(calls, loc); /* Succeed or throw */
    known = 1;
  }
  TRY_CLEANUP {
    xbt_free(loc);
  }
  CATCH(ex) {
    if (ex.category != not_found_error)
      RETHROW;
    xbt_ex_free(ex);
  }
  return known;
}

void* smpi_shared_get_call(const char* func, const char* input) {
   char* loc = bprintf("%s:%s", func, input);
   void* data;

   if(!calls) {
      calls = xbt_dict_new_homogeneous(NULL);
   }
   data = xbt_dict_get(calls, loc);
   free(loc);
   return data;
}

void* smpi_shared_set_call(const char* func, const char* input, void* data) {
   char* loc = bprintf("%s:%s", func, input);

   if(!calls) {
      calls = xbt_dict_new_homogeneous(NULL);
   }
   xbt_dict_set(calls, loc, data, NULL);
   free(loc);
   return data;
}




#define TOPAGE(addr) (void *)(((unsigned long)(addr) / xbt_pagesize) * xbt_pagesize)


/** Map a given SMPI privatization segment (make a SMPI process active)
 */
void smpi_switch_data_segment(int dest){

  if (smpi_loaded_page==dest)//no need to switch either
   return;

  // So the job:
  smpi_really_switch_data_segment(dest);
}

/** Map a given SMPI privatization segment (make a SMPI process active)
 *  even if SMPI thinks it is already active
 *
 *  When doing a state restoration, the state of the restored variables
 *  might not be consistent with the state of the virtual memory.
 *  In this case, we to change the data segment.
 */
void smpi_really_switch_data_segment(int dest) {

  if(size_data_exe == 0)//no need to switch
    return;

#ifdef HAVE_MMAP
  int i;
  if(smpi_loaded_page==-1){//initial switch, do the copy from the real page here
    for (i=0; i< SIMIX_process_count(); i++){
      memcpy(smpi_privatisation_regions[i].address,TOPAGE(start_data_exe),size_data_exe);
    }
  }

  int current = smpi_privatisation_regions[dest].file_descriptor;
  XBT_DEBUG("Switching data frame to the one of process %d", dest);
  void* tmp = mmap (TOPAGE(start_data_exe), size_data_exe, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, current, 0);
  if (tmp != TOPAGE(start_data_exe))
    xbt_die("Couldn't map the new region");
  smpi_loaded_page=dest;
#endif
}

int smpi_is_privatisation_file(char* file)
{
  return strncmp("/dev/shm/my-buffer-", file, 19) == 0;
}

void smpi_get_executable_global_size(){
  int size_bss_binary=0;
  int size_data_binary=0;
  FILE *fp;
  char *line = NULL;            /* Temporal storage for each line that is readed */
  ssize_t read;                 /* Number of bytes readed */
  size_t n = 0;                 /* Amount of bytes to read by xbt_getline */

  char *lfields[7];
  int i, found = 0;

  char *command = bprintf("objdump --section-headers %s", xbt_binary_name);

  fp = popen(command, "r");

  if(fp == NULL){
    perror("popen failed");
    xbt_abort();
  }

  while ((read = xbt_getline(&line, &n, fp)) != -1 && found != 2) {

    if(n == 0)
      continue;

    /* Wipeout the new line character */
    line[read - 1] = '\0';

    lfields[0] = strtok(line, " ");

    if(lfields[0] == NULL)
      continue;

    if(strcmp(lfields[0], "Sections:") == 0
        || strcmp(lfields[0], "Idx") == 0
        || strncmp(lfields[0], xbt_binary_name, strlen(xbt_binary_name)) == 0)
      continue;

    for (i = 1; i < 7 && lfields[i - 1] != NULL; i++) {
      lfields[i] = strtok(NULL, " ");
    }

    /*
     * we are looking for these fields
    23 .data         02625a20  00000000006013e0  00000000006013e0  000013e0  2**5
                     CONTENTS, ALLOC, LOAD, DATA
    24 .bss          02625a40  0000000002c26e00  0000000002c26e00  02626e00  2**5
                     ALLOC
    */

    if(i>=6){
      if(strcmp(lfields[1], ".data") == 0){
        size_data_binary = strtoul(lfields[2], NULL, 16);
        start_data_exe = (char*) strtoul(lfields[4], NULL, 16);
        found++;
      }else if(strcmp(lfields[1], ".bss") == 0){
        //the beginning of bss is not exactly the end of data if not aligned, grow bss reported size accordingly
        //TODO : check if this is OK, as some segments may be inserted between them..
        size_bss_binary = ((char*) strtoul(lfields[4], NULL, 16) - (start_data_exe + size_data_binary))
                          + strtoul(lfields[2], NULL, 16);
        found++;
       }

    }

  }

  size_data_exe =(unsigned long)start_data_exe - (unsigned long)TOPAGE(start_data_exe)+ size_data_binary+size_bss_binary;
  xbt_free(command);
  xbt_free(line);
  pclose(fp);

}

void smpi_initialize_global_memory_segments(){

#ifndef HAVE_MMAP
  smpi_privatize_global_variables=0;
  return;
#else

  unsigned int i = 0;
  smpi_get_executable_global_size();

  XBT_DEBUG ("bss+data segment found : size %d starting at %p",size_data_exe, start_data_exe );

  if(size_data_exe == 0){//no need to switch
    smpi_privatize_global_variables=0;
    return;
  }

  smpi_privatisation_regions = (smpi_privatisation_region_t) malloc(
    smpi_process_count() * sizeof(struct s_smpi_privatisation_region));

  for (i=0; i< SIMIX_process_count(); i++){
      //create SIMIX_process_count() mappings of this size with the same data inside
      void *address = NULL;
      char path[] = "/dev/shm/my-buffer-XXXXXX";
      int status;

      int file_descriptor= mkstemp (path);
      if (file_descriptor < 0)
        xbt_die("Impossible to create temporary file for memory mapping");

      status = unlink (path);
      if (status)
        xbt_die("Impossible to unlink temporary file for memory mapping");

      status = ftruncate(file_descriptor, size_data_exe);
      if(status)
        xbt_die("Impossible to set the size of the temporary file for memory mapping");

      /* Ask for a free region */
      address = mmap (NULL, size_data_exe, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
      if (address == MAP_FAILED)
        xbt_die("Couldn't find a free region for memory mapping");

      //initialize the values
      memcpy(address,TOPAGE(start_data_exe),size_data_exe);

      //store the address of the mapping for further switches
      smpi_privatisation_regions[i].file_descriptor = file_descriptor;
      smpi_privatisation_regions[i].address = address;
  }

#endif

}

void smpi_destroy_global_memory_segments(){
  if(size_data_exe == 0)//no need to switch
    return;
#ifdef HAVE_MMAP
  int i;
  for (i=0; i< smpi_process_count(); i++){
    if(munmap(smpi_privatisation_regions[i].address,size_data_exe) < 0) {
      XBT_WARN("Unmapping of fd %d failed: %s",
        smpi_privatisation_regions[i].file_descriptor, strerror(errno));
    }
    close(smpi_privatisation_regions[i].file_descriptor);
  }
  xbt_free(smpi_privatisation_regions);
#endif

}
