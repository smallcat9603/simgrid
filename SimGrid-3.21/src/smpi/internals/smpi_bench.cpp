/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "getopt.h"
#include "private.hpp"
#include "simgrid/host.h"
#include "simgrid/modelchecker.h"
#include "simgrid/s4u/Exec.hpp"
#include "smpi_comm.hpp"
#include "src/internal_config.h"
#include "src/mc/mc_replay.hpp"
#include "src/simix/ActorImpl.hpp"
#include "xbt/config.hpp"

#include "src/smpi/include/smpi_actor.hpp"
#include <unordered_map>

#ifndef WIN32
#include <sys/mman.h>
#endif
#include <cmath>

#if HAVE_PAPI
#include <papi.h>
#endif

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(smpi_bench, smpi, "Logging specific to SMPI (benchmarking)");

static simgrid::config::Flag<double>
    smpi_wtime_sleep("smpi/wtime",
                     "Minimum time to inject inside a call to MPI_Wtime(), gettimeofday() and clock_gettime()",
                     1e-8 /* Documented to be 10 ns */);

double smpi_cpu_threshold = -1;
double smpi_host_speed;

SharedMallocType smpi_cfg_shared_malloc = SharedMallocType::GLOBAL;
double smpi_total_benched_time = 0;

extern "C" XBT_PUBLIC void smpi_execute_flops_(double* flops);

void smpi_execute_flops_(double *flops)
{
  smpi_execute_flops(*flops);
}

extern "C" XBT_PUBLIC void smpi_execute_(double* duration);
void smpi_execute_(double *duration)
{
  smpi_execute(*duration);
}

void smpi_execute_flops(double flops) {
  xbt_assert(flops >= 0, "You're trying to execute a negative amount of flops (%f)!", flops);
  XBT_DEBUG("Handle real computation time: %f flops", flops);
  simgrid::s4u::this_actor::exec_init(flops)
      ->set_name("computation")
      ->set_tracing_category(TRACE_internal_smpi_get_category())
      ->start()
      ->wait();
  smpi_switch_data_segment(simgrid::s4u::Actor::self());
}

void smpi_execute(double duration)
{
  if (duration >= smpi_cpu_threshold) {
    XBT_DEBUG("Sleep for %g to handle real computation time", duration);
    double flops = duration * smpi_host_speed;
    int rank     = simgrid::s4u::this_actor::get_pid();
    TRACE_smpi_computing_in(rank, flops);

    smpi_execute_flops(flops);

    TRACE_smpi_computing_out(rank);

  } else {
    XBT_DEBUG("Real computation took %g while option smpi/cpu-threshold is set to %g => ignore it", duration,
              smpi_cpu_threshold);
  }
}

void smpi_execute_benched(double duration)
{
  smpi_bench_end();
  double speed = sg_host_speed(sg_host_self());
  smpi_execute_flops(duration*speed);
  smpi_bench_begin();
}

void smpi_bench_begin()
{
  if (smpi_privatize_global_variables == SmpiPrivStrategies::MMAP) {
    smpi_switch_data_segment(simgrid::s4u::Actor::self());
  }

  if (MC_is_active() || MC_record_replay_is_active())
    return;

#if HAVE_PAPI
  if (not simgrid::config::get_value<std::string>("smpi/papi-events").empty()) {
    int event_set = smpi_process()->papi_event_set();
    // PAPI_start sets everything to 0! See man(3) PAPI_start
    if (PAPI_LOW_LEVEL_INITED == PAPI_is_initialized()) {
      if (PAPI_start(event_set) != PAPI_OK) {
        // TODO This needs some proper handling.
        XBT_CRITICAL("Could not start PAPI counters.\n");
        xbt_die("Error.");
      }
    }
  }
#endif
  xbt_os_threadtimer_start(smpi_process()->timer());
}

void smpi_bench_end()
{
  if (MC_is_active() || MC_record_replay_is_active())
    return;

  double speedup = 1;
  xbt_os_timer_t timer = smpi_process()->timer();
  xbt_os_threadtimer_stop(timer);

#if HAVE_PAPI
  /**
   * An MPI function has been called and now is the right time to update
   * our PAPI counters for this process.
   */
  if (not simgrid::config::get_value<std::string>("smpi/papi-events").empty()) {
    papi_counter_t& counter_data        = smpi_process()->papi_counters();
    int event_set                       = smpi_process()->papi_event_set();
    std::vector<long long> event_values = std::vector<long long>(counter_data.size());

    if (PAPI_stop(event_set, &event_values[0]) != PAPI_OK) { // Error
      XBT_CRITICAL("Could not stop PAPI counters.\n");
      xbt_die("Error.");
    } else {
      for (unsigned int i = 0; i < counter_data.size(); i++) {
        counter_data[i].second += event_values[i];
      }
    }
  }
#endif

  if (smpi_process()->sampling()) {
    XBT_CRITICAL("Cannot do recursive benchmarks.");
    XBT_CRITICAL("Are you trying to make a call to MPI within a SMPI_SAMPLE_ block?");
    xbt_backtrace_display_current();
    xbt_die("Aborting.");
  }

  // Maybe we need to artificially speed up or slow down our computation based on our statistical analysis.
  if (simgrid::config::get_value<std::string>("smpi/comp-adjustment-file")[0] != '\0') {

    smpi_trace_call_location_t* loc                            = smpi_process()->call_location();
    std::string key                                            = loc->get_composed_key();
    std::unordered_map<std::string, double>::const_iterator it = location2speedup.find(key);
    if (it != location2speedup.end()) {
      speedup = it->second;
    }
  }

  // Simulate the benchmarked computation unless disabled via command-line argument
  if (simgrid::config::get_value<bool>("smpi/simulate-computation")) {
    smpi_execute(xbt_os_timer_elapsed(timer)/speedup);
  }

#if HAVE_PAPI
  if (not simgrid::config::get_value<std::string>("smpi/papi-events").empty() && TRACE_smpi_is_enabled()) {
    container_t container =
        simgrid::instr::Container::by_name(std::string("rank-") + std::to_string(simgrid::s4u::this_actor::get_pid()));
    papi_counter_t& counter_data = smpi_process()->papi_counters();

    for (auto const& pair : counter_data) {
      simgrid::instr::VariableType* variable = static_cast<simgrid::instr::VariableType*>(container->type_->by_name(pair.first));
      variable->set_event(SIMIX_get_clock(), pair.second);
    }
  }
#endif

  smpi_total_benched_time += xbt_os_timer_elapsed(timer);
}

/* Private sleep function used by smpi_sleep(), smpi_usleep() and friends */
static unsigned int private_sleep(double secs)
{
  smpi_bench_end();

  XBT_DEBUG("Sleep for: %lf secs", secs);
  int rank = simgrid::s4u::this_actor::get_pid();
  TRACE_smpi_sleeping_in(rank, secs);

  simcall_process_sleep(secs);

  TRACE_smpi_sleeping_out(rank);

  smpi_bench_begin();
  return 0;
}

unsigned int smpi_sleep(unsigned int secs)
{
  if (not smpi_process())
    return sleep(secs);
  return private_sleep(static_cast<double>(secs));
}

int smpi_usleep(useconds_t usecs)
{
  if (not smpi_process())
    return usleep(usecs);
  return static_cast<int>(private_sleep(static_cast<double>(usecs) / 1000000.0));
}

#if _POSIX_TIMERS > 0
int smpi_nanosleep(const struct timespec* tp, struct timespec* t)
{
  if (not smpi_process())
    return nanosleep(tp,t);
  return static_cast<int>(private_sleep(static_cast<double>(tp->tv_sec + tp->tv_nsec / 1000000000.0)));
}
#endif

int smpi_gettimeofday(struct timeval* tv, struct timezone* tz)
{
  if (not smpi_process())
    return gettimeofday(tv, tz);

  smpi_bench_end();
  double now = SIMIX_get_clock();
  if (tv) {
    tv->tv_sec = static_cast<time_t>(now);
#ifdef WIN32
    tv->tv_usec = static_cast<useconds_t>((now - tv->tv_sec) * 1e6);
#else
    tv->tv_usec = static_cast<suseconds_t>((now - tv->tv_sec) * 1e6);
#endif
  }
  if (smpi_wtime_sleep > 0)
    simcall_process_sleep(smpi_wtime_sleep);
  smpi_bench_begin();
  return 0;
}

#if _POSIX_TIMERS > 0
int smpi_clock_gettime(clockid_t clk_id, struct timespec* tp)
{
  if (not smpi_process())
    return clock_gettime(clk_id, tp);
  //there is only one time in SMPI, so clk_id is ignored.
  smpi_bench_end();
  double now = SIMIX_get_clock();
  if (tp) {
    tp->tv_sec = static_cast<time_t>(now);
    tp->tv_nsec = static_cast<long int>((now - tp->tv_sec) * 1e9);
  }
  if (smpi_wtime_sleep > 0)
    simcall_process_sleep(smpi_wtime_sleep);
  smpi_bench_begin();
  return 0;
}
#endif

double smpi_mpi_wtime()
{
  double time;
  if (smpi_process()->initialized() && not smpi_process()->finalized() && not smpi_process()->sampling()) {
    smpi_bench_end();
    time = SIMIX_get_clock();
    if (smpi_wtime_sleep > 0)
      simcall_process_sleep(smpi_wtime_sleep);
    smpi_bench_begin();
  } else {
    time = SIMIX_get_clock();
  }
  return time;
}

extern double sg_surf_precision;
unsigned long long smpi_rastro_resolution ()
{
  smpi_bench_end();
  double resolution = (1/sg_surf_precision);
  smpi_bench_begin();
  return static_cast<unsigned long long>(resolution);
}

unsigned long long smpi_rastro_timestamp ()
{
  smpi_bench_end();
  double now = SIMIX_get_clock();

  unsigned long long sec = static_cast<unsigned long long>(now);
  unsigned long long pre = (now - sec) * smpi_rastro_resolution();
  smpi_bench_begin();
  return static_cast<unsigned long long>(sec) * smpi_rastro_resolution() + pre;
}

/* ****************************** Functions related to the SMPI_SAMPLE_ macros ************************************/
namespace {
class SampleLocation : public std::string {
public:
  SampleLocation(bool global, const char* file, int line) : std::string(std::string(file) + ":" + std::to_string(line))
  {
    if (not global)
      this->append(":" + std::to_string(simgrid::s4u::this_actor::get_pid()));
  }
};

class LocalData {
public:
  double threshold; /* maximal stderr requested (if positive) */
  double relstderr; /* observed stderr so far */
  double mean;      /* mean of benched times, to be used if the block is disabled */
  double sum;       /* sum of benched times (to compute the mean and stderr) */
  double sum_pow2;  /* sum of the square of the benched times (to compute the stderr) */
  int iters;        /* amount of requested iterations */
  int count;        /* amount of iterations done so far */
  bool benching;    /* true: we are benchmarking; false: we have enough data, no bench anymore */

  bool need_more_benchs() const;
};

bool LocalData::need_more_benchs() const
{
  bool res = (count < iters) || (threshold > 0.0 && (count < 2 ||          // not enough data
                                                     relstderr > threshold // stderr too high yet
                                                     ));
  XBT_DEBUG("%s (count:%d iter:%d stderr:%f thres:%f mean:%fs)",
            (res ? "need more data" : "enough benchs"), count, iters, relstderr, threshold, mean);
  return res;
}

std::unordered_map<SampleLocation, LocalData, std::hash<std::string>> samples;
}

void smpi_sample_1(int global, const char *file, int line, int iters, double threshold)
{
  SampleLocation loc(global, file, line);

  smpi_bench_end();     /* Take time from previous, unrelated computation into account */
  smpi_process()->set_sampling(1);

  auto insert = samples.emplace(loc, LocalData{
                                         threshold, // threshold
                                         0.0,       // relstderr
                                         0.0,       // mean
                                         0.0,       // sum
                                         0.0,       // sum_pow2
                                         iters,     // iters
                                         0,         // count
                                         true       // benching (if we have no data, we need at least one)
                                     });
  LocalData& data = insert.first->second;
  if (insert.second) {
    XBT_DEBUG("XXXXX First time ever on benched nest %s.", loc.c_str());
    xbt_assert(threshold > 0 || iters > 0,
        "You should provide either a positive amount of iterations to bench, or a positive maximal stderr (or both)");
  } else {
    if (data.iters != iters || data.threshold != threshold) {
      XBT_ERROR("Asked to bench block %s with different settings %d, %f is not %d, %f. "
                "How did you manage to give two numbers at the same line??",
                loc.c_str(), data.iters, data.threshold, iters, threshold);
      THROW_IMPOSSIBLE;
    }

    // if we already have some data, check whether sample_2 should get one more bench or whether it should emulate
    // the computation instead
    data.benching = data.need_more_benchs();
    XBT_DEBUG("XXXX Re-entering the benched nest %s. %s", loc.c_str(),
              (data.benching ? "more benching needed" : "we have enough data, skip computes"));
  }
}

int smpi_sample_2(int global, const char *file, int line)
{
  SampleLocation loc(global, file, line);
  int res;

  XBT_DEBUG("sample2 %s", loc.c_str());
  auto sample = samples.find(loc);
  if (sample == samples.end())
    xbt_die("Y U NO use SMPI_SAMPLE_* macros? Stop messing directly with smpi_sample_* functions!");
  LocalData& data = sample->second;

  if (data.benching) {
    // we need to run a new bench
    XBT_DEBUG("benchmarking: count:%d iter:%d stderr:%f thres:%f; mean:%f",
              data.count, data.iters, data.relstderr, data.threshold, data.mean);
    res = 1;
  } else {
    // Enough data, no more bench (either we got enough data from previous visits to this benched nest, or we just
    //ran one bench and need to bail out now that our job is done). Just sleep instead
    XBT_DEBUG("No benchmark (either no need, or just ran one): count >= iter (%d >= %d) or stderr<thres (%f<=%f)."
              " apply the %fs delay instead",
              data.count, data.iters, data.relstderr, data.threshold, data.mean);
    smpi_execute(data.mean);
    smpi_process()->set_sampling(0);
    res = 0; // prepare to capture future, unrelated computations
  }
  smpi_bench_begin();
  return res;
}

void smpi_sample_3(int global, const char *file, int line)
{
  SampleLocation loc(global, file, line);

  XBT_DEBUG("sample3 %s", loc.c_str());
  auto sample = samples.find(loc);
  if (sample == samples.end())
    xbt_die("Y U NO use SMPI_SAMPLE_* macros? Stop messing directly with smpi_sample_* functions!");
  LocalData& data = sample->second;

  if (not data.benching)
    THROW_IMPOSSIBLE;

  // ok, benchmarking this loop is over
  xbt_os_threadtimer_stop(smpi_process()->timer());

  // update the stats
  data.count++;
  double period  = xbt_os_timer_elapsed(smpi_process()->timer());
  data.sum      += period;
  data.sum_pow2 += period * period;
  double n       = static_cast<double>(data.count);
  data.mean      = data.sum / n;
  data.relstderr = sqrt((data.sum_pow2 / n - data.mean * data.mean) / n) / data.mean;
  if (data.need_more_benchs()) {
    data.mean = period; // Still in benching process; We want sample_2 to simulate the exact time of this loop
    // occurrence before leaving, not the mean over the history
  }
  XBT_DEBUG("Average mean after %d steps is %f, relative standard error is %f (sample was %f)",
            data.count, data.mean, data.relstderr, period);

  // That's enough for now, prevent sample_2 to run the same code over and over
  data.benching = false;
}

smpi_trace_call_location_t* smpi_trace_get_call_location()
{
  return smpi_process()->call_location();
}

void smpi_trace_set_call_location(const char* file, const int line)
{
  smpi_trace_call_location_t* loc = smpi_process()->call_location();

  loc->previous_filename   = loc->filename;
  loc->previous_linenumber = loc->linenumber;
  loc->filename            = file;
  loc->linenumber          = line;
}

/** Required for Fortran bindings */
void smpi_trace_set_call_location_(const char* file, int* line)
{
  smpi_trace_set_call_location(file, *line);
}

/** Required for Fortran if -fsecond-underscore is activated */
void smpi_trace_set_call_location__(const char* file, int* line)
{
  smpi_trace_set_call_location(file, *line);
}

void smpi_bench_destroy()
{
  samples.clear();
}

int smpi_getopt_long_only (int argc,  char *const *argv,  const char *options,
                      const struct option * long_options, int *opt_index)
{
  if (smpi_process())
    optind = smpi_process()->get_optind();
  int ret = getopt_long_only (argc,  argv,  options, long_options, opt_index);
  if (smpi_process())
    smpi_process()->set_optind(optind);
  return ret;
}

int smpi_getopt_long (int argc,  char *const *argv,  const char *options,
                      const struct option * long_options, int *opt_index)
{
  if (smpi_process())
    optind = smpi_process()->get_optind();
  int ret = getopt_long (argc,  argv,  options, long_options, opt_index);
  if (smpi_process())
    smpi_process()->set_optind(optind);
  return ret;
}

int smpi_getopt (int argc,  char *const *argv,  const char *options)
{
  if (smpi_process())
    optind = smpi_process()->get_optind();
  int ret = getopt (argc,  argv,  options);
  if (smpi_process())
    smpi_process()->set_optind(optind);
  return ret;
}
