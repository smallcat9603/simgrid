/* Functions related to the Virtual Machines.                               */

/* Copyright (c) 2012-2018. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef MSG_VM_H
#define MSG_VM_H

#include <jni.h>
#include "simgrid/msg.h"

SG_BEGIN_DECL()

/* Shut up some errors in eclipse online compiler. I wish such a pimple wouldn't be needed */
#ifndef JNIEXPORT
#define JNIEXPORT
#endif
#ifndef JNICALL
#define JNICALL
#endif
/* end of eclipse-mandated pimple */

void jvm_bind(JNIEnv *env, jobject jvm, msg_vm_t vm);
msg_vm_t jvm_get_native(JNIEnv *env, jobject jvm);

JNIEXPORT void JNICALL Java_org_simgrid_msg_VM_nativeInit(JNIEnv* env, jclass cls);

JNIEXPORT jobjectArray JNICALL Java_org_simgrid_msg_VM_all(JNIEnv* env, jclass cls_arg);

JNIEXPORT jint JNICALL Java_org_simgrid_msg_VM_isCreated(JNIEnv* env, jobject jvm);
JNIEXPORT jint JNICALL Java_org_simgrid_msg_VM_isRunning(JNIEnv* env, jobject jvm);
JNIEXPORT jint JNICALL Java_org_simgrid_msg_VM_isMigrating(JNIEnv* env, jobject jvm);
JNIEXPORT jint JNICALL Java_org_simgrid_msg_VM_isSuspended(JNIEnv* env, jobject jvm);
JNIEXPORT jint JNICALL Java_org_simgrid_msg_VM_isResuming(JNIEnv* env, jobject jvm);

JNIEXPORT void JNICALL Java_org_simgrid_msg_VM_setBound(JNIEnv* env, jobject jvm, jdouble bound);

JNIEXPORT void JNICALL Java_org_simgrid_msg_VM_create(JNIEnv* env, jobject jvm, jobject jhost, jstring jname,
                                                      jint coreAmount, jint jramsize, jint dprate, jint mig_netspeed);
JNIEXPORT jobject JNICALL Java_org_simgrid_msg_VM_getVMByName(JNIEnv* env, jclass cls, jstring jname);
JNIEXPORT void JNICALL Java_org_simgrid_msg_VM_nativeFinalize(JNIEnv* env, jobject jvm);
JNIEXPORT void JNICALL Java_org_simgrid_msg_VM_start(JNIEnv* env, jobject jvm);
JNIEXPORT void JNICALL Java_org_simgrid_msg_VM_nativeMigration(JNIEnv* env, jobject jvm, jobject jhost);
JNIEXPORT void JNICALL Java_org_simgrid_msg_VM_suspend(JNIEnv* env, jobject jvm);
JNIEXPORT void JNICALL Java_org_simgrid_msg_VM_resume(JNIEnv* env, jobject jvm);
JNIEXPORT void JNICALL Java_org_simgrid_msg_VM_shutdown(JNIEnv* env, jobject jvm);
JNIEXPORT void JNICALL Java_org_simgrid_msg_VM_destroy(JNIEnv* env, jobject jvm);

SG_END_DECL()

#endif
