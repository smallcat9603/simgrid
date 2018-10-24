/* Java Wrappers to the MSG API.                                            */

/* Copyright (c) 2007-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <algorithm>
#include <clocale>
#include <string>

#include "simgrid/Exception.hpp"
#include "simgrid/msg.h"
#include "simgrid/plugins/energy.h"
#include "simgrid/plugins/file_system.h"
#include "simgrid/plugins/live_migration.h"
#include "simgrid/simix.h"

#include "simgrid/s4u/Host.hpp"

#include "src/simix/smx_private.hpp"

#include "jmsg.hpp"
#include "jmsg_as.hpp"
#include "jmsg_host.h"
#include "jmsg_process.h"
#include "jmsg_storage.h"
#include "jmsg_task.h"
#include "jxbt_utilities.hpp"

#include "JavaContext.hpp"


/* Shut up some errors in eclipse online compiler. I wish such a pimple wouldn't be needed */
#ifndef JNIEXPORT
#define JNIEXPORT
#endif
#ifndef JNICALL
#define JNICALL
#endif
/* end of eclipse-mandated pimple */

int JAVA_HOST_LEVEL = -1;

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(java);

JavaVM *__java_vm = nullptr;

JNIEnv *get_current_thread_env()
{
  using simgrid::kernel::context::JavaContext;
  JavaContext* ctx = static_cast<JavaContext*>(xbt_os_thread_get_extra_data());
  return ctx->jenv_;
}

void jmsg_throw_status(JNIEnv *env, msg_error_t status) {
  switch (status) {
    case MSG_TIMEOUT:
      jxbt_throw_time_out_failure(env, "");
      break;
    case MSG_TRANSFER_FAILURE:
      jxbt_throw_transfer_failure(env, "");
      break;
    case MSG_HOST_FAILURE:
      jxbt_throw_host_failure(env, "");
      break;
    case MSG_TASK_CANCELED:
      jxbt_throw_task_cancelled(env, "");
      break;
    default:
      xbt_die("undefined message failed (please see jmsg_throw_status function in jmsg.cpp)");
  }
}

/***************************************************************************************
 * Unsortable functions                                                        *
 ***************************************************************************************/

JNIEXPORT jdouble JNICALL Java_org_simgrid_msg_Msg_getClock(JNIEnv * env, jclass cls)
{
  return (jdouble) MSG_get_clock();
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Msg_init(JNIEnv * env, jclass cls, jobjectArray jargs)
{
  int argc = 0;

  env->GetJavaVM(&__java_vm);

  simgrid::kernel::context::factory_initializer = &simgrid::kernel::context::java_factory;
  jthrowable exc = env->ExceptionOccurred();
  if (exc) {
    env->ExceptionClear();
  }

  setlocale(LC_NUMERIC,"C");

  if (jargs)
    argc = static_cast<int>(env->GetArrayLength(jargs));

  argc++;
  char** argv = new char*[argc + 1];
  argv[0] = xbt_strdup("java");

  for (int index = 0; index < argc - 1; index++) {
    jstring jval    = (jstring)env->GetObjectArrayElement(jargs, index);
    const char* tmp = env->GetStringUTFChars(jval, 0);
    argv[index + 1] = xbt_strdup(tmp);
    env->ReleaseStringUTFChars(jval, tmp);
  }
  argv[argc] = nullptr;

  MSG_init(&argc, argv);
  sg_vm_live_migration_plugin_init();

  JAVA_HOST_LEVEL = simgrid::s4u::Host::extension_create(nullptr);

  for (int index = 0; index < argc - 1; index++) {
    env->SetObjectArrayElement(jargs, index, (jstring)env->NewStringUTF(argv[index + 1]));
    free(argv[index]);
  }
  free(argv[argc]);
  delete[] argv;
}

JNIEXPORT void JNICALL JNICALL Java_org_simgrid_msg_Msg_run(JNIEnv * env, jclass cls)
{
  /* Run everything */
  XBT_DEBUG("Ready to run MSG_MAIN");
  msg_error_t rv = MSG_main();
  XBT_DEBUG("Done running MSG_MAIN");
  jxbt_check_res("MSG_main()", rv, MSG_OK,
                 xbt_strdup("unexpected error : MSG_main() failed .. please report this bug "));

  XBT_INFO("MSG_main finished; Terminating the simulation...");
  /* Cleanup java hosts */
  xbt_dynar_t hosts = MSG_hosts_as_dynar();
  for (unsigned long index = 0; index < xbt_dynar_length(hosts) - 1; index++) {
    msg_host_t msg_host = xbt_dynar_get_as(hosts,index,msg_host_t);
    jobject jhost = (jobject) msg_host->extension(JAVA_HOST_LEVEL);
    if (jhost)
      jhost_unref(env, jhost);
  }
  xbt_dynar_free(&hosts);

  /* Cleanup java storages */
  for (auto const& elm : java_storage_map)
    jstorage_unref(env, elm.second);

  /* FIXME: don't be of such an EXTREM BRUTALITY to stop the jvm. Sorry I don't get it working otherwise.
   * See the comment in ActorImpl.cpp::SIMIX_process_kill() */
  exit(0);
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Msg_createEnvironment(JNIEnv * env, jclass cls, jstring jplatformFile)
{
  const char *platformFile = env->GetStringUTFChars(jplatformFile, 0);

  MSG_create_environment(platformFile);

  env->ReleaseStringUTFChars(jplatformFile, platformFile);
}

JNIEXPORT jobject JNICALL Java_org_simgrid_msg_Msg_environmentGetRoutingRoot(JNIEnv * env, jclass cls)
{
  msg_netzone_t as = MSG_zone_get_root();
  jobject jas      = jnetzone_new_instance(env);
  if (not jas) {
    jxbt_throw_jni(env, "java As instantiation failed");
    return nullptr;
  }
  jas = jnetzone_ref(env, jas);
  if (not jas) {
    jxbt_throw_jni(env, "new global ref allocation failed");
    return nullptr;
  }
  jnetzone_bind(jas, as, env);

  return (jobject) jas;
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Msg_debug(JNIEnv * env, jclass cls, jstring js)
{
  const char *s = env->GetStringUTFChars(js, 0);
  XBT_DEBUG("%s", s);
  env->ReleaseStringUTFChars(js, s);
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Msg_verb(JNIEnv * env, jclass cls, jstring js)
{
  const char *s = env->GetStringUTFChars(js, 0);
  XBT_VERB("%s", s);
  env->ReleaseStringUTFChars(js, s);
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Msg_info(JNIEnv * env, jclass cls, jstring js)
{
  const char *s = env->GetStringUTFChars(js, 0);
  XBT_INFO("%s", s);
  env->ReleaseStringUTFChars(js, s);
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Msg_warn(JNIEnv * env, jclass cls, jstring js)
{
  const char *s = env->GetStringUTFChars(js, 0);
  XBT_WARN("%s", s);
  env->ReleaseStringUTFChars(js, s);
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Msg_error(JNIEnv * env, jclass cls, jstring js)
{
  const char *s = env->GetStringUTFChars(js, 0);
  XBT_ERROR("%s", s);
  env->ReleaseStringUTFChars(js, s);
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Msg_critical(JNIEnv * env, jclass cls, jstring js)
{
  const char *s = env->GetStringUTFChars(js, 0);
  XBT_CRITICAL("%s", s);
  env->ReleaseStringUTFChars(js, s);
}

static int java_main(int argc, char *argv[]);

JNIEXPORT void JNICALL
Java_org_simgrid_msg_Msg_deployApplication(JNIEnv * env, jclass cls, jstring jdeploymentFile)
{
  const char *deploymentFile = env->GetStringUTFChars(jdeploymentFile, 0);

  SIMIX_function_register_default(java_main);
  MSG_launch_application(deploymentFile);
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Msg_energyInit() {
  sg_host_energy_plugin_init();
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Msg_fileSystemInit()
{
  sg_storage_file_system_init();
}

/** Run a Java org.simgrid.msg.Process
 *
 *  If needed, this waits for the process starting time.
 *  Then it calls the Process.run() method.
 */
static void run_jprocess(JNIEnv *env, jobject jprocess)
{
  // wait for the process's start time
  jfieldID jprocess_field_Process_startTime = jxbt_get_sfield(env, "org/simgrid/msg/Process", "startTime", "D");
  jdouble startTime = env->GetDoubleField(jprocess, jprocess_field_Process_startTime);
  if (startTime > MSG_get_clock())
    MSG_process_sleep(startTime - MSG_get_clock());

  //Execution of the "run" method.
  jmethodID id = jxbt_get_smethod(env, "org/simgrid/msg/Process", "run", "()V");
  xbt_assert((id != nullptr), "Method Process.run() not found...");

  env->CallVoidMethod(jprocess, id);
}

/** Create a Java org.simgrid.msg.Process with the arguments and run it */
static int java_main(int argc, char *argv[])
{
  JNIEnv *env = get_current_thread_env();
  simgrid::kernel::context::JavaContext* context = static_cast<simgrid::kernel::context::JavaContext*>(SIMIX_context_self());

  //Change the "." in class name for "/".
  std::string arg0 = argv[0];
  std::replace(begin(arg0), end(arg0), '.', '/');
  jclass class_Process = env->FindClass(arg0.c_str());
  //Retrieve the methodID for the constructor
  xbt_assert((class_Process != nullptr), "Class not found (%s). The deployment file must use the fully qualified class name, including the package. The case is important.", argv[0]);
  jmethodID constructor_Process = env->GetMethodID(class_Process, "<init>", "(Lorg/simgrid/msg/Host;Ljava/lang/String;[Ljava/lang/String;)V");
  xbt_assert((constructor_Process != nullptr), "Constructor not found for class %s. Is there a (Host, String ,String[]) constructor in your class ?", argv[0]);

  //Retrieve the name of the process.
  jstring jname = env->NewStringUTF(argv[0]);
  //Build the arguments
  jobjectArray args = static_cast<jobjectArray>(env->NewObjectArray(argc - 1, env->FindClass("java/lang/String"),
                                                                    env->NewStringUTF("")));
  for (int i = 1; i < argc; i++)
      env->SetObjectArrayElement(args,i - 1, env->NewStringUTF(argv[i]));
  //Retrieve the host for the process.
  jstring jhostName = env->NewStringUTF(MSG_host_self()->get_cname());
  jobject jhost = Java_org_simgrid_msg_Host_getByName(env, nullptr, jhostName);
  //creates the process
  jobject jprocess = env->NewObject(class_Process, constructor_Process, jhost, jname, args);
  xbt_assert((jprocess != nullptr), "Process allocation failed.");
  jprocess = env->NewGlobalRef(jprocess);
  //bind the process to the context
  msg_process_t process = MSG_process_self();

  context->jprocess_ = jprocess;
  /* sets the PID and the PPID of the process */
  env->SetIntField(jprocess, jprocess_field_Process_pid, static_cast<jint>(MSG_process_get_PID(process)));
  env->SetIntField(jprocess, jprocess_field_Process_ppid, static_cast<jint>(MSG_process_get_PPID(process)));
  jprocess_bind(jprocess, process, env);

  run_jprocess(env, context->jprocess_);
  return 0;
}

namespace simgrid {
namespace kernel {
namespace context {

/** Run the Java org.simgrid.msg.Process */
void java_main_jprocess(jobject jprocess)
{
  JNIEnv *env = get_current_thread_env();
  simgrid::kernel::context::JavaContext* context = static_cast<simgrid::kernel::context::JavaContext*>(SIMIX_context_self());
  context->jprocess_                             = jprocess;
  jprocess_bind(context->jprocess_, MSG_process_self(), env);

  run_jprocess(env, context->jprocess_);
}
}}}
