/* Functions related to the java process instances.                         */

/* Copyright (c) 2007-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "jmsg_process.h"

#include "JavaContext.hpp"
#include "jmsg.hpp"
#include "jmsg_host.h"
#include "jxbt_utilities.hpp"
#include "simgrid/Exception.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(java);

jfieldID jprocess_field_Process_bind;
jfieldID jprocess_field_Process_host;
jfieldID jprocess_field_Process_killTime;
jfieldID jprocess_field_Process_name;
jfieldID jprocess_field_Process_pid;
jfieldID jprocess_field_Process_ppid;

jobject jprocess_from_native(const_sg_actor_t actor)
{
  const simgrid::kernel::context::JavaContext* context =
      static_cast<simgrid::kernel::context::JavaContext*>(actor->get_impl()->context_.get());
  return context->jprocess_;
}

jobject jprocess_ref(jobject jprocess, JNIEnv* env)
{
  return env->NewGlobalRef(jprocess);
}

void jprocess_unref(jobject jprocess, JNIEnv* env)
{
  env->DeleteGlobalRef(jprocess);
}

sg_actor_t jprocess_to_native(jobject jprocess, JNIEnv* env)
{
  return (sg_actor_t)(intptr_t)env->GetLongField(jprocess, jprocess_field_Process_bind);
}

void jprocess_bind(jobject jprocess, const_sg_actor_t actor, JNIEnv* env)
{
  env->SetLongField(jprocess, jprocess_field_Process_bind, (intptr_t)actor);
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_nativeInit(JNIEnv *env, jclass cls) {
  jclass jprocess_class_Process = env->FindClass("org/simgrid/msg/Process");
  xbt_assert(jprocess_class_Process, "Native initialization of msg/Process failed. Please report that bug");

  jprocess_field_Process_name = jxbt_get_jfield(env, jprocess_class_Process, "name", "Ljava/lang/String;");
  jprocess_field_Process_bind = jxbt_get_jfield(env, jprocess_class_Process, "bind", "J");
  jprocess_field_Process_pid = jxbt_get_jfield(env, jprocess_class_Process, "pid", "I");
  jprocess_field_Process_ppid = jxbt_get_jfield(env, jprocess_class_Process, "ppid", "I");
  jprocess_field_Process_host = jxbt_get_jfield(env, jprocess_class_Process, "host", "Lorg/simgrid/msg/Host;");
  jprocess_field_Process_killTime = jxbt_get_jfield(env, jprocess_class_Process, "killTime", "D");
  xbt_assert(jprocess_field_Process_name && jprocess_field_Process_pid && jprocess_field_Process_ppid &&
                 jprocess_field_Process_host,
             "Native initialization of msg/Process failed. Please report that bug");
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_create(JNIEnv* env, jobject jprocess_arg, jobject jhost)
{
  /* create a global java process instance */
  jobject jprocess = jprocess_ref(jprocess_arg, env);

  /* Actually build the MSG process */
  auto jname        = (jstring)env->GetObjectField(jprocess, jprocess_field_Process_name);
  const char* name  = env->GetStringUTFChars(jname, nullptr);
  auto actor_code   = [jprocess]() { simgrid::kernel::context::java_main_jprocess(jprocess); };
  smx_actor_t self  = SIMIX_process_self();
  sg_host_t host    = jhost_get_native(env, jhost);
  smx_actor_t actor = simgrid::kernel::actor::simcall([name, actor_code, host, self] {
    return simgrid::kernel::actor::ActorImpl::create(std::move(name), std::move(actor_code), nullptr, host, nullptr,
                                                     self)
        .get();
  });
  sg_actor_yield();

  env->ReleaseStringUTFChars(jname, name);

  /* Retrieve the kill time from the actor */
  actor->get_ciface()->set_kill_time((double)env->GetDoubleField(jprocess, jprocess_field_Process_killTime));

  /* sets the PID and the PPID of the actor */
  env->SetIntField(jprocess, jprocess_field_Process_pid, (jint)actor->get_ciface()->get_pid());
  env->SetIntField(jprocess, jprocess_field_Process_ppid, (jint)actor->get_ciface()->get_ppid());
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_daemonize(JNIEnv* env, jobject jprocess)
{
  sg_actor_t actor = jprocess_to_native(jprocess, env);

  if (not actor) {
    jxbt_throw_notbound(env, "process", jprocess);
    return;
  }

  actor->daemonize();
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_killAll(JNIEnv* env, jclass cls)
{
  sg_actor_kill_all();
}

JNIEXPORT jobject JNICALL Java_org_simgrid_msg_Process_fromPID(JNIEnv * env, jclass cls, jint pid)
{
  auto const* actor = sg_actor_by_PID(pid);

  if (not actor) {
    jxbt_throw_process_not_found(env, std::string("PID = ") + std::to_string(static_cast<int>(pid)));
    return nullptr;
  }

  jobject jprocess = jprocess_from_native(actor);

  if (not jprocess) {
    jxbt_throw_jni(env, "get process failed");
    return nullptr;
  }

  return jprocess;
}

JNIEXPORT jint JNICALL Java_org_simgrid_msg_Process_nativeGetPID(JNIEnv* env, jobject jprocess)
{
  const_sg_actor_t actor = jprocess_to_native(jprocess, env);
  return sg_actor_get_PID(actor);
}

JNIEXPORT jobject JNICALL Java_org_simgrid_msg_Process_getProperty(JNIEnv *env, jobject jprocess, jobject jname) {
  const_sg_actor_t actor = jprocess_to_native(jprocess, env);

  if (not actor) {
    jxbt_throw_notbound(env, "process", jprocess);
    return nullptr;
  }
  const char* name = env->GetStringUTFChars((jstring)jname, nullptr);

  const char* property = actor->get_property(name);
  if (not property)
    return nullptr;

  jobject jproperty = env->NewStringUTF(property);

  env->ReleaseStringUTFChars((jstring)jname, name);

  return jproperty;
}

JNIEXPORT jobject JNICALL Java_org_simgrid_msg_Process_getCurrentProcess(JNIEnv * env, jclass cls)
{
  jobject jprocess = jprocess_from_native(sg_actor_self());
  if (not jprocess)
    jxbt_throw_jni(env, xbt_strdup("SIMIX_process_get_jprocess() failed"));

  return jprocess;
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_suspend(JNIEnv * env, jobject jprocess)
{
  sg_actor_t actor = jprocess_to_native(jprocess, env);

  if (not actor) {
    jxbt_throw_notbound(env, "process", jprocess);
    return;
  }

  /* suspend the process */
  actor->suspend();
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_resume(JNIEnv * env, jobject jprocess)
{
  sg_actor_t actor = jprocess_to_native(jprocess, env);

  if (not actor) {
    jxbt_throw_notbound(env, "process", jprocess);
    return;
  }

  /* resume the process */
  actor->resume();
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_setAutoRestart(JNIEnv* env, jobject jprocess,
                                                                   jboolean jauto_restart)
{
  sg_actor_t actor = jprocess_to_native(jprocess, env);
  if (not actor) {
    jxbt_throw_notbound(env, "process", jprocess);
    return;
  }

  actor->set_auto_restart(jauto_restart == JNI_TRUE);
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_restart (JNIEnv *env, jobject jprocess) {
  sg_actor_t actor = jprocess_to_native(jprocess, env);

  if (not actor) {
    jxbt_throw_notbound(env, "process", jprocess);
    return;
  }

  actor->restart();
}

JNIEXPORT jboolean JNICALL Java_org_simgrid_msg_Process_isSuspended(JNIEnv * env, jobject jprocess)
{
  sg_actor_t actor = jprocess_to_native(jprocess, env);

  if (not actor) {
    jxbt_throw_notbound(env, "process", jprocess);
    return 0;
  }

  /* true is the actor is suspended, false otherwise */
  return (jboolean)actor->is_suspended();
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_sleep(JNIEnv *env, jclass cls, jlong jmillis, jint jnanos)
 {
  double time =  ((double)jmillis) / 1000 + ((double)jnanos) / 1000000000;
  msg_error_t rv = MSG_OK;
  if (not simgrid::ForcefulKillException::try_n_catch([&time]() { simgrid::s4u::this_actor::sleep_for(time); })) {
    rv = MSG_HOST_FAILURE;
  }
  if (rv != MSG_OK) {
    jmsg_throw_status(env, rv);
  }
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_waitFor(JNIEnv * env, jobject jprocess, jdouble jseconds)
{
  msg_error_t rv = MSG_OK;
  if (not simgrid::ForcefulKillException::try_n_catch(
          [&jseconds]() { simgrid::s4u::this_actor::sleep_for((double)jseconds); })) {
    rv = MSG_HOST_FAILURE;
    jxbt_throw_by_name(env, "org/simgrid/msg/ProcessKilledError", "Process killed");
  }
  if (env->ExceptionOccurred())
    return;
  if (rv != MSG_OK) {
    XBT_DEBUG("Status NOK");
    jmsg_throw_status(env,rv);
  }
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_kill(JNIEnv * env, jobject jprocess)
{
  /* get the native instances from the java ones */
  sg_actor_t actor = jprocess_to_native(jprocess, env);
  if (not actor) {
    jxbt_throw_notbound(env, "process", jprocess);
    return;
  }
  if (not simgrid::ForcefulKillException::try_n_catch([&actor]() { actor->kill(); })) {
    jxbt_throw_by_name(env, "org/simgrid/msg/ProcessKilledError", "Process killed");
  }
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_migrate(JNIEnv * env, jobject jprocess, jobject jhost)
{
  sg_actor_t actor = jprocess_to_native(jprocess, env);

  if (not actor) {
    jxbt_throw_notbound(env, "process", jprocess);
    return;
  }

  sg_host_t host = jhost_get_native(env, jhost);

  if (not host) {
    jxbt_throw_notbound(env, "host", jhost);
    return;
  }

  /* change the host of the actor */
  actor->set_host(host);

  /* change the host java side */
  env->SetObjectField(jprocess, jprocess_field_Process_host, jhost);
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_yield(JNIEnv* env, jclass cls)
{
  simgrid::s4u::this_actor::yield();
}

JNIEXPORT void JNICALL Java_org_simgrid_msg_Process_setKillTime (JNIEnv *env , jobject jprocess, jdouble jkilltime) {
  jprocess_to_native(jprocess, env)->set_kill_time((double)jkilltime);
}

JNIEXPORT jint JNICALL Java_org_simgrid_msg_Process_getCount(JNIEnv * env, jclass cls) {
  return (jint)sg_actor_count();
}
