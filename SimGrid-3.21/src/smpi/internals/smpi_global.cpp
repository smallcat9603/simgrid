/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "mc/mc.h"
#include "simgrid/s4u/Engine.hpp"
#include "smpi_coll.hpp"
#include "smpi_f2c.hpp"
#include "smpi_host.hpp"
#include "src/kernel/activity/CommImpl.hpp"
#include "src/simix/smx_private.hpp"
#include "src/smpi/include/smpi_actor.hpp"
#include "xbt/config.hpp"

#include <algorithm>
#include <cfloat> /* DBL_MAX */
#include <dlfcn.h>
#include <fcntl.h>
#include <fstream>

#if not defined(__APPLE__)
#include <link.h>
#endif

#if defined(__APPLE__)
# include <AvailabilityMacros.h>
# ifndef MAC_OS_X_VERSION_10_12
#   define MAC_OS_X_VERSION_10_12 101200
# endif
# define HAVE_WORKING_MMAP (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12)
#elif defined(__FreeBSD__)
# define HAVE_WORKING_MMAP 0
#else
# define HAVE_WORKING_MMAP 1
#endif

#if HAVE_SENDFILE
#include <sys/sendfile.h>
#endif

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(smpi_kernel, smpi, "Logging specific to SMPI (kernel)");
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp> /* trim_right / trim_left */

#if SMPI_IFORT
  extern "C" void for_rtl_init_ (int *, char **);
  extern "C" void for_rtl_finish_ ();
#elif SMPI_FLANG
  extern "C" void __io_set_argc(int);
  extern "C" void __io_set_argv(char **);
#elif SMPI_GFORTRAN
  extern "C" void _gfortran_set_args(int, char **);
#endif

/* RTLD_DEEPBIND is a bad idea of GNU ld that obviously does not exist on other platforms
 * See https://www.akkadia.org/drepper/dsohowto.pdf
 * and https://lists.freebsd.org/pipermail/freebsd-current/2016-March/060284.html
*/
#if !defined(RTLD_DEEPBIND) || HAVE_SANITIZER_ADDRESS || HAVE_SANITIZER_THREAD
#define WANT_RTLD_DEEPBIND 0
#else
#define WANT_RTLD_DEEPBIND RTLD_DEEPBIND
#endif

#if HAVE_PAPI
#include "papi.h"
std::string papi_default_config_name = "default";
std::map</* computation unit name */ std::string, papi_process_data> units2papi_setup;
#endif

using simgrid::s4u::Actor;
using simgrid::s4u::ActorPtr;
std::unordered_map<std::string, double> location2speedup;

static std::map</*process_id*/ ActorPtr, simgrid::smpi::ActorExt*> process_data;
int process_count = 0;
static int smpi_exit_status = 0;
int smpi_universe_size = 0;
extern double smpi_total_benched_time;
xbt_os_timer_t global_timer;
static std::vector<std::string> privatize_libs_paths;
/**
 * Setting MPI_COMM_WORLD to MPI_COMM_UNINITIALIZED (it's a variable)
 * is important because the implementation of MPI_Comm checks
 * "this == MPI_COMM_UNINITIALIZED"? If yes, it uses smpi_process()->comm_world()
 * instead of "this".
 * This is basically how we only have one global variable but all processes have
 * different communicators (the one their SMPI instance uses).
 *
 * See smpi_comm.cpp and the functions therein for details.
 */
MPI_Comm MPI_COMM_WORLD = MPI_COMM_UNINITIALIZED;
MPI_Errhandler *MPI_ERRORS_RETURN = nullptr;
MPI_Errhandler *MPI_ERRORS_ARE_FATAL = nullptr;
MPI_Errhandler *MPI_ERRHANDLER_NULL = nullptr;
// No instance gets manually created; check also the smpirun.in script as
// this default name is used there as well (when the <actor> tag is generated).
static const std::string smpi_default_instance_name("smpirun");
static simgrid::config::Flag<double> smpi_init_sleep(
  "smpi/init", "Time to inject inside a call to MPI_Init", 0.0);

void (*smpi_comm_copy_data_callback) (smx_activity_t, void*, size_t) = &smpi_comm_copy_buffer_callback;

int smpi_process_count()
{
  return process_count;
}

simgrid::smpi::ActorExt* smpi_process()
{
  ActorPtr me = Actor::self();

  if (me == nullptr) // This happens sometimes (eg, when linking against NS3 because it pulls openMPI...)
    return nullptr;

  return process_data.at(me);
}

simgrid::smpi::ActorExt* smpi_process_remote(ActorPtr actor)
{
  return process_data.at(actor);
}

MPI_Comm smpi_process_comm_self(){
  return smpi_process()->comm_self();
}

void smpi_process_init(int *argc, char ***argv){
  simgrid::smpi::ActorExt::init(argc, argv);
}

int smpi_process_index(){
  return simgrid::s4u::this_actor::get_pid();
}

void * smpi_process_get_user_data(){
  return Actor::self()->get_impl()->get_user_data();
}

void smpi_process_set_user_data(void *data){
  Actor::self()->get_impl()->set_user_data(data);
}


int smpi_global_size()
{
  char *value = getenv("SMPI_GLOBAL_SIZE");
  xbt_assert(value,"Please set env var SMPI_GLOBAL_SIZE to the expected number of processes.");

  return xbt_str_parse_int(value, "SMPI_GLOBAL_SIZE contains a non-numerical value: %s");
}

void smpi_comm_set_copy_data_callback(void (*callback) (smx_activity_t, void*, size_t))
{
  smpi_comm_copy_data_callback = callback;
}

static void memcpy_private(void* dest, const void* src, std::vector<std::pair<size_t, size_t>>& private_blocks)
{
  for (auto const& block : private_blocks)
    memcpy((uint8_t*)dest+block.first, (uint8_t*)src+block.first, block.second-block.first);
}

static void check_blocks(std::vector<std::pair<size_t, size_t>> &private_blocks, size_t buff_size) {
  for (auto const& block : private_blocks)
    xbt_assert(block.first <= block.second && block.second <= buff_size, "Oops, bug in shared malloc.");
}

void smpi_comm_copy_buffer_callback(smx_activity_t synchro, void *buff, size_t buff_size)
{
  simgrid::kernel::activity::CommImplPtr comm =
      boost::dynamic_pointer_cast<simgrid::kernel::activity::CommImpl>(synchro);
  int src_shared                        = 0;
  int dst_shared                        = 0;
  size_t src_offset                     = 0;
  size_t dst_offset                     = 0;
  std::vector<std::pair<size_t, size_t>> src_private_blocks;
  std::vector<std::pair<size_t, size_t>> dst_private_blocks;
  XBT_DEBUG("Copy the data over");
  if((src_shared=smpi_is_shared(buff, src_private_blocks, &src_offset))) {
    XBT_DEBUG("Sender %p is shared. Let's ignore it.", buff);
    src_private_blocks = shift_and_frame_private_blocks(src_private_blocks, src_offset, buff_size);
  }
  else {
    src_private_blocks.clear();
    src_private_blocks.push_back(std::make_pair(0, buff_size));
  }
  if((dst_shared=smpi_is_shared((char*)comm->dst_buff, dst_private_blocks, &dst_offset))) {
    XBT_DEBUG("Receiver %p is shared. Let's ignore it.", (char*)comm->dst_buff);
    dst_private_blocks = shift_and_frame_private_blocks(dst_private_blocks, dst_offset, buff_size);
  }
  else {
    dst_private_blocks.clear();
    dst_private_blocks.push_back(std::make_pair(0, buff_size));
  }
  check_blocks(src_private_blocks, buff_size);
  check_blocks(dst_private_blocks, buff_size);
  auto private_blocks = merge_private_blocks(src_private_blocks, dst_private_blocks);
  check_blocks(private_blocks, buff_size);
  void* tmpbuff=buff;
  if ((smpi_privatize_global_variables == SmpiPrivStrategies::MMAP) &&
      (static_cast<char*>(buff) >= smpi_data_exe_start) &&
      (static_cast<char*>(buff) < smpi_data_exe_start + smpi_data_exe_size)) {
    XBT_DEBUG("Privatization : We are copying from a zone inside global memory... Saving data to temp buffer !");
    smpi_switch_data_segment(comm->src_proc->iface());
    tmpbuff = static_cast<void*>(xbt_malloc(buff_size));
    memcpy_private(tmpbuff, buff, private_blocks);
  }

  if ((smpi_privatize_global_variables == SmpiPrivStrategies::MMAP) && ((char*)comm->dst_buff >= smpi_data_exe_start) &&
      ((char*)comm->dst_buff < smpi_data_exe_start + smpi_data_exe_size)) {
    XBT_DEBUG("Privatization : We are copying to a zone inside global memory - Switch data segment");
    smpi_switch_data_segment(comm->dst_proc->iface());
  }
  XBT_DEBUG("Copying %zu bytes from %p to %p", buff_size, tmpbuff,comm->dst_buff);
  memcpy_private(comm->dst_buff, tmpbuff, private_blocks);

  if (comm->detached) {
    // if this is a detached send, the source buffer was duplicated by SMPI
    // sender to make the original buffer available to the application ASAP
    xbt_free(buff);
    //It seems that the request is used after the call there this should be free somewhere else but where???
    //xbt_free(comm->comm.src_data);// inside SMPI the request is kept inside the user data and should be free
    comm->src_buff = nullptr;
  }
  if (tmpbuff != buff)
    xbt_free(tmpbuff);
}

void smpi_comm_null_copy_buffer_callback(smx_activity_t comm, void *buff, size_t buff_size)
{
  /* nothing done in this version */
}

static void smpi_check_options()
{
  //check correctness of MPI parameters

  xbt_assert(simgrid::config::get_value<int>("smpi/async-small-thresh") <=
             simgrid::config::get_value<int>("smpi/send-is-detached-thresh"));

  if (simgrid::config::is_default("smpi/host-speed")) {
    XBT_INFO("You did not set the power of the host running the simulation.  "
             "The timings will certainly not be accurate.  "
             "Use the option \"--cfg=smpi/host-speed:<flops>\" to set its value."
             "Check http://simgrid.org/simgrid/latest/doc/options.html#options_smpi_bench for more information.");
  }

  xbt_assert(simgrid::config::get_value<double>("smpi/cpu-threshold") >= 0,
             "The 'smpi/cpu-threshold' option cannot have negative values [anymore]. If you want to discard "
             "the simulation of any computation, please use 'smpi/simulate-computation:no' instead.");
}

int smpi_enabled() {
  return not process_data.empty();
}

void smpi_global_init()
{
  if (not MC_is_active()) {
    global_timer = xbt_os_timer_new();
    xbt_os_walltimer_start(global_timer);
  }

  std::string filename = simgrid::config::get_value<std::string>("smpi/comp-adjustment-file");
  if (not filename.empty()) {
    std::ifstream fstream(filename);
    if (not fstream.is_open()) {
      xbt_die("Could not open file %s. Does it exist?", filename.c_str());
    }

    std::string line;
    typedef boost::tokenizer< boost::escaped_list_separator<char>> Tokenizer;
    std::getline(fstream, line); // Skip the header line
    while (std::getline(fstream, line)) {
      Tokenizer tok(line);
      Tokenizer::iterator it  = tok.begin();
      Tokenizer::iterator end = std::next(tok.begin());

      std::string location = *it;
      boost::trim(location);
      location2speedup.insert(std::pair<std::string, double>(location, std::stod(*end)));
    }
  }

#if HAVE_PAPI
  // This map holds for each computation unit (such as "default" or "process1" etc.)
  // the configuration as given by the user (counter data as a pair of (counter_name, counter_counter))
  // and the (computed) event_set.

  if (not simgrid::config::get_value<std::string>("smpi/papi-events").empty()) {
    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT)
      XBT_ERROR("Could not initialize PAPI library; is it correctly installed and linked?"
                " Expected version is %u", PAPI_VER_CURRENT);

    typedef boost::tokenizer<boost::char_separator<char>> Tokenizer;
    boost::char_separator<char> separator_units(";");
    std::string str = simgrid::config::get_value<std::string>("smpi/papi-events");
    Tokenizer tokens(str, separator_units);

    // Iterate over all the computational units. This could be processes, hosts, threads, ranks... You name it.
    // I'm not exactly sure what we will support eventually, so I'll leave it at the general term "units".
    for (auto const& unit_it : tokens) {
      boost::char_separator<char> separator_events(":");
      Tokenizer event_tokens(unit_it, separator_events);

      int event_set = PAPI_NULL;
      if (PAPI_create_eventset(&event_set) != PAPI_OK) {
        // TODO: Should this let the whole simulation die?
        XBT_CRITICAL("Could not create PAPI event set during init.");
      }

      // NOTE: We cannot use a map here, as we must obey the order of the counters
      // This is important for PAPI: We need to map the values of counters back
      // to the event_names (so, when PAPI_read() has finished)!
      papi_counter_t counters2values;

      // Iterate over all counters that were specified for this specific
      // unit.
      // Note that we need to remove the name of the unit
      // (that could also be the "default" value), which always comes first.
      // Hence, we start at ++(events.begin())!
      for (Tokenizer::iterator events_it = ++(event_tokens.begin()); events_it != event_tokens.end(); ++events_it) {

        int event_code   = PAPI_NULL;
        char* event_name = const_cast<char*>((*events_it).c_str());
        if (PAPI_event_name_to_code(event_name, &event_code) == PAPI_OK) {
          if (PAPI_add_event(event_set, event_code) != PAPI_OK) {
            XBT_ERROR("Could not add PAPI event '%s'. Skipping.", event_name);
            continue;
          } else {
            XBT_DEBUG("Successfully added PAPI event '%s' to the event set.", event_name);
          }
        } else {
          XBT_CRITICAL("Could not find PAPI event '%s'. Skipping.", event_name);
          continue;
        }

        counters2values.push_back(
            // We cannot just pass *events_it, as this is of type const basic_string
            std::make_pair<std::string, long long>(std::string(*events_it), 0));
      }

      std::string unit_name    = *(event_tokens.begin());
      papi_process_data config = {.counter_data = std::move(counters2values), .event_set = event_set};

      units2papi_setup.insert(std::make_pair(unit_name, std::move(config)));
    }
  }
#endif
}

void smpi_global_destroy()
{
  smpi_bench_destroy();
  smpi_shared_destroy();
  smpi_deployment_cleanup_instances();

  if (simgrid::smpi::Colls::smpi_coll_cleanup_callback != nullptr)
    simgrid::smpi::Colls::smpi_coll_cleanup_callback();

  MPI_COMM_WORLD = MPI_COMM_NULL;

  if (not MC_is_active()) {
    xbt_os_timer_free(global_timer);
  }

  if (smpi_privatize_global_variables == SmpiPrivStrategies::MMAP)
    smpi_destroy_global_memory_segments();
  smpi_free_static();
  if(simgrid::smpi::F2C::lookup() != nullptr)
    simgrid::smpi::F2C::delete_lookup();
}

static void smpi_init_options(){
  // return if already called
  if (smpi_cpu_threshold > -1)
    return;
  simgrid::smpi::Colls::set_collectives();
  simgrid::smpi::Colls::smpi_coll_cleanup_callback = nullptr;
  smpi_cpu_threshold                               = simgrid::config::get_value<double>("smpi/cpu-threshold");
  smpi_host_speed                                  = simgrid::config::get_value<double>("smpi/host-speed");
  xbt_assert(smpi_host_speed > 0.0, "You're trying to set the host_speed to a non-positive value (given: %f)", smpi_host_speed);
  std::string smpi_privatize_option = simgrid::config::get_value<std::string>("smpi/privatization");
  if (smpi_privatize_option == "no" || smpi_privatize_option == "0")
    smpi_privatize_global_variables = SmpiPrivStrategies::NONE;
  else if (smpi_privatize_option == "yes" || smpi_privatize_option == "1")
    smpi_privatize_global_variables = SmpiPrivStrategies::DEFAULT;
  else if (smpi_privatize_option == "mmap")
    smpi_privatize_global_variables = SmpiPrivStrategies::MMAP;
  else if (smpi_privatize_option == "dlopen")
    smpi_privatize_global_variables = SmpiPrivStrategies::DLOPEN;
  else
    xbt_die("Invalid value for smpi/privatization: '%s'", smpi_privatize_option.c_str());

  if (not SMPI_switch_data_segment) {
    XBT_DEBUG("Running without smpi_main(); disable smpi/privatization.");
    smpi_privatize_global_variables = SmpiPrivStrategies::NONE;
  }
#if !HAVE_WORKING_MMAP
  if (smpi_privatize_global_variables == SmpiPrivStrategies::MMAP) {
    XBT_INFO("mmap privatization is broken on this platform, switching to dlopen privatization instead.");
    smpi_privatize_global_variables = SmpiPrivStrategies::DLOPEN;
  }
#endif

  if (smpi_cpu_threshold < 0)
    smpi_cpu_threshold = DBL_MAX;

  std::string val = simgrid::config::get_value<std::string>("smpi/shared-malloc");
  if ((val == "yes") || (val == "1") || (val == "on") || (val == "global")) {
    smpi_cfg_shared_malloc = SharedMallocType::GLOBAL;
  } else if (val == "local") {
    smpi_cfg_shared_malloc = SharedMallocType::LOCAL;
  } else if ((val == "no") || (val == "0") || (val == "off")) {
    smpi_cfg_shared_malloc = SharedMallocType::NONE;
  } else {
    xbt_die("Invalid value '%s' for option smpi/shared-malloc. Possible values: 'on' or 'global', 'local', 'off'",
            val.c_str());
  }
}

typedef std::function<int(int argc, char *argv[])> smpi_entry_point_type;
typedef int (* smpi_c_entry_point_type)(int argc, char **argv);
typedef void (*smpi_fortran_entry_point_type)();

static int smpi_run_entry_point(smpi_entry_point_type entry_point, std::vector<std::string> args)
{
  // copy C strings, we need them writable
  std::vector<char*>* args4argv = new std::vector<char*>(args.size());
  std::transform(begin(args), end(args), begin(*args4argv), [](const std::string& s) { return xbt_strdup(s.c_str()); });

#if !SMPI_IFORT
  // take a copy of args4argv to keep reference of the allocated strings
  const std::vector<char*> args2str(*args4argv);
#endif
  int argc = args4argv->size();
  args4argv->push_back(nullptr);
  char** argv = args4argv->data();

  simgrid::smpi::ActorExt::init(&argc, &argv);
#if SMPI_IFORT
  for_rtl_init_ (&argc, argv);
#elif SMPI_FLANG
  __io_set_argc(argc);
  __io_set_argv(argv);
#elif SMPI_GFORTRAN
  _gfortran_set_args(argc, argv);
#endif 
  int res = entry_point(argc, argv);

#if SMPI_IFORT
  for_rtl_finish_ ();
#else
  for (char* s : args2str)
    xbt_free(s);
  delete args4argv;
#endif

  if (res != 0){
    XBT_WARN("SMPI process did not return 0. Return value : %d", res);
    if (smpi_exit_status == 0)
      smpi_exit_status = res;
  }
  return 0;
}


// TODO, remove the number of functions involved here
static smpi_entry_point_type smpi_resolve_function(void* handle)
{
  smpi_fortran_entry_point_type entry_point_fortran = (smpi_fortran_entry_point_type)dlsym(handle, "user_main_");
  if (entry_point_fortran != nullptr) {
    return [entry_point_fortran](int argc, char** argv) {
      entry_point_fortran();
      return 0;
    };
  }

  smpi_c_entry_point_type entry_point = (smpi_c_entry_point_type)dlsym(handle, "main");
  if (entry_point != nullptr) {
    return entry_point;
  }

  return smpi_entry_point_type();
}

static void smpi_copy_file(std::string src, std::string target, off_t fdin_size)
{
  int fdin = open(src.c_str(), O_RDONLY);
  xbt_assert(fdin >= 0, "Cannot read from %s. Please make sure that the file exists and is executable.", src.c_str());
  int fdout = open(target.c_str(), O_CREAT | O_RDWR, S_IRWXU);
  xbt_assert(fdout >= 0, "Cannot write into %s", target.c_str());

  XBT_DEBUG("Copy %ld bytes into %s", static_cast<long>(fdin_size), target.c_str());
#if HAVE_SENDFILE
  ssize_t sent_size = sendfile(fdout, fdin, NULL, fdin_size);
  xbt_assert(sent_size == fdin_size, "Error while copying %s: only %zd bytes copied instead of %ld (errno: %d -- %s)",
             target.c_str(), sent_size, fdin_size, errno, strerror(errno));
#else
  const int bufsize = 1024 * 1024 * 4;
  char buf[bufsize];
  while (int got = read(fdin, buf, bufsize)) {
    if (got == -1) {
      xbt_assert(errno == EINTR, "Cannot read from %s", src.c_str());
    } else {
      char* p  = buf;
      int todo = got;
      while (int done = write(fdout, p, todo)) {
        if (done == -1) {
          xbt_assert(errno == EINTR, "Cannot write into %s", target.c_str());
        } else {
          p += done;
          todo -= done;
        }
      }
    }
  }
#endif
  close(fdin);
  close(fdout);
}

#if not defined(__APPLE__)
static int visit_libs(struct dl_phdr_info* info, size_t, void* data)
{
  char* libname = (char*)(data);
  const char *path = info->dlpi_name;
  if(strstr(path, libname)){
    strncpy(libname, path, 512);
    return 1;
  }
  
  return 0;
}
#endif

static void smpi_init_privatization_dlopen(std::string executable)
{
  // Prepare the copy of the binary (get its size)
  struct stat fdin_stat;
  stat(executable.c_str(), &fdin_stat);
  off_t fdin_size         = fdin_stat.st_size;
  static std::size_t rank = 0;

  std::string libnames = simgrid::config::get_value<std::string>("smpi/privatize-libs");
  if (not libnames.empty()) {
    // split option
    std::vector<std::string> privatize_libs;
    boost::split(privatize_libs, libnames, boost::is_any_of(";"));

    for (auto const& libname : privatize_libs) {
      // load the library once to add it to the local libs, to get the absolute path
      void* libhandle = dlopen(libname.c_str(), RTLD_LAZY);
      // get library name from path
      char fullpath[512] = {'\0'};
      strcpy(fullpath, libname.c_str());
#if not defined(__APPLE__)
      int ret = dl_iterate_phdr(visit_libs, fullpath);
      if (ret == 0)
        xbt_die("Can't find a linked %s - check the setting you gave to smpi/privatize-libs", fullpath);
      else
        XBT_DEBUG("Extra lib to privatize found : %s", fullpath);
#else
      xbt_die("smpi/privatize-libs is not (yet) compatible with OSX");
#endif
      privatize_libs_paths.push_back(fullpath);
      dlclose(libhandle);
    }
  }

  simix_global->default_function = [executable, fdin_size](std::vector<std::string> args) {
    return std::function<void()>([executable, fdin_size, args] {

      // Copy the dynamic library:
      std::string target_executable =
          executable + "_" + std::to_string(getpid()) + "_" + std::to_string(rank) + ".so";

      smpi_copy_file(executable, target_executable, fdin_size);
      // if smpi/privatize-libs is set, duplicate pointed lib and link each executable copy to a different one.
      std::vector<std::string> target_libs;
      for (auto const& libpath : privatize_libs_paths) {
        // if we were given a full path, strip it
        size_t index = libpath.find_last_of("/\\");
        std::string libname;
        if (index != std::string::npos)
          libname = libpath.substr(index + 1);

        if (not libname.empty()) {
          // load the library to add it to the local libs, to get the absolute path
          struct stat fdin_stat2;
          stat(libpath.c_str(), &fdin_stat2);
          off_t fdin_size2 = fdin_stat2.st_size;

          // Copy the dynamic library, the new name must be the same length as the old one
          // just replace the name with 7 digits for the rank and the rest of the name.
          unsigned int pad = 7;
          if (libname.length() < pad)
            pad = libname.length();
          std::string target_lib =
              std::string(pad - std::to_string(rank).length(), '0') + std::to_string(rank) + libname.substr(pad);
          target_libs.push_back(target_lib);
          XBT_DEBUG("copy lib %s to %s, with size %lld", libpath.c_str(), target_lib.c_str(), (long long)fdin_size2);
          smpi_copy_file(libpath, target_lib, fdin_size2);

          std::string sedcommand = "sed -i -e 's/" + libname + "/" + target_lib + "/g' " + target_executable;
          int ret                = system(sedcommand.c_str());
          if (ret != 0)
            xbt_die("error while applying sed command %s \n", sedcommand.c_str());
        }
      }

      rank++;
      // Load the copy and resolve the entry point:
      void* handle    = dlopen(target_executable.c_str(), RTLD_LAZY | RTLD_LOCAL | WANT_RTLD_DEEPBIND);
      int saved_errno = errno;
      if (simgrid::config::get_value<bool>("smpi/keep-temps") == false) {
        unlink(target_executable.c_str());
        for (const std::string& target_lib : target_libs)
          unlink(target_lib.c_str());
      }
      if (handle == nullptr)
        xbt_die("dlopen failed: %s (errno: %d -- %s)", dlerror(), saved_errno, strerror(saved_errno));
      smpi_entry_point_type entry_point = smpi_resolve_function(handle);
      if (not entry_point)
        xbt_die("Could not resolve entry point");
      smpi_run_entry_point(entry_point, args);
    });
  };
}

static void smpi_init_privatization_no_dlopen(std::string executable)
{
  if (smpi_privatize_global_variables == SmpiPrivStrategies::MMAP)
    smpi_prepare_global_memory_segment();
  // Load the dynamic library and resolve the entry point:
  void* handle = dlopen(executable.c_str(), RTLD_LAZY | RTLD_LOCAL);
  if (handle == nullptr)
    xbt_die("dlopen failed for %s: %s (errno: %d -- %s)", executable.c_str(), dlerror(), errno, strerror(errno));
  smpi_entry_point_type entry_point = smpi_resolve_function(handle);
  if (not entry_point)
    xbt_die("main not found in %s", executable.c_str());
  if (smpi_privatize_global_variables == SmpiPrivStrategies::MMAP)
    smpi_backup_global_memory_segment();

  // Execute the same entry point for each simulated process:
  simix_global->default_function = [entry_point](std::vector<std::string> args) {
    return std::function<void()>([entry_point, args] { smpi_run_entry_point(entry_point, args); });
  };
}

int smpi_main(const char* executable, int argc, char* argv[])
{
  srand(SMPI_RAND_SEED);

  if (getenv("SMPI_PRETEND_CC") != nullptr) {
    /* Hack to ensure that smpicc can pretend to be a simple compiler. Particularly handy to pass it to the
     * configuration tools */
    return 0;
  }

  TRACE_global_init();
  SIMIX_global_init(&argc, argv);

  SMPI_switch_data_segment = &smpi_switch_data_segment;

  // TODO This will not be executed in the case where smpi_main is not called,
  // e.g., not for smpi_msg_masterslave. This should be moved to another location
  // that is always called -- maybe close to Actor::on_creation?
  simgrid::s4u::Host::on_creation.connect(
      [](simgrid::s4u::Host& host) { host.extension_set(new simgrid::smpi::Host(&host)); });

  // parse the platform file: get the host list
  simgrid::s4u::Engine::get_instance()->load_platform(argv[1]);
  SIMIX_comm_set_copy_data_callback(smpi_comm_copy_buffer_callback);

  smpi_init_options();
  if (smpi_privatize_global_variables == SmpiPrivStrategies::DLOPEN)
    smpi_init_privatization_dlopen(executable);
  else
    smpi_init_privatization_no_dlopen(executable);

  SMPI_init();
  simgrid::s4u::Engine::get_instance()->load_deployment(argv[2]);
  SMPI_app_instance_register(smpi_default_instance_name.c_str(), nullptr,
                             process_data.size()); // This call has a side effect on process_count...
  MPI_COMM_WORLD = *smpi_deployment_comm_world(smpi_default_instance_name);
  smpi_universe_size = process_count;


  /* Clean IO before the run */
  fflush(stdout);
  fflush(stderr);

  if (MC_is_active()) {
    MC_run();
  } else {

    SIMIX_run();

    xbt_os_walltimer_stop(global_timer);
    if (simgrid::config::get_value<bool>("smpi/display-timing")) {
      double global_time = xbt_os_timer_elapsed(global_timer);
      XBT_INFO("Simulated time: %g seconds. \n\n"
          "The simulation took %g seconds (after parsing and platform setup)\n"
          "%g seconds were actual computation of the application",
          SIMIX_get_clock(), global_time , smpi_total_benched_time);

      if (smpi_total_benched_time/global_time>=0.75)
      XBT_INFO("More than 75%% of the time was spent inside the application code.\n"
      "You may want to use sampling functions or trace replay to reduce this.");
    }
  }
  smpi_global_destroy();

  return smpi_exit_status;
}

// Called either directly from the user code, or from the code called by smpirun
void SMPI_init(){
  simgrid::s4u::Actor::on_creation.connect([](simgrid::s4u::ActorPtr actor) {
    if (not actor->is_daemon()) {
      process_data.insert({actor, new simgrid::smpi::ActorExt(actor, nullptr)});
    }
  });
  simgrid::s4u::Actor::on_destruction.connect([](simgrid::s4u::ActorPtr actor) {
    auto it = process_data.find(actor);
    if (it != process_data.end()) {
      delete it->second;
      process_data.erase(it);
    }
  });

  smpi_init_options();
  smpi_global_init();
  smpi_check_options();
}

void SMPI_finalize(){
  smpi_global_destroy();
}

void smpi_mpi_init() {
  smpi_init_fortran_types();
  if(smpi_init_sleep > 0)
    simcall_process_sleep(smpi_init_sleep);
}
