/* Functions related to the java comm instances                             */

/* Copyright (c) 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef MSG_JCOMM_H
#define MSG_JCOMM_H
#include <jni.h>
#include <msg/msg.h>
/**
 * This function binds the task associated with the communication to
 * the java communication object.
 */
void jcomm_bind_task(JNIEnv *env, jobject jcomm);

JNIEXPORT void JNICALL
Java_org_simgrid_msg_Comm_nativeInit(JNIEnv *env, jclass cls);

JNIEXPORT void JNICALL
Java_org_simgrid_msg_Comm_destroy(JNIEnv *env, jobject jcomm);

JNIEXPORT jboolean JNICALL
Java_org_simgrid_msg_Comm_test(JNIEnv *env, jobject jcomm);

JNIEXPORT void JNICALL
Java_org_simgrid_msg_Comm_waitCompletion(JNIEnv *env, jobject jcomm, jdouble timeout);
#endif /* MSG_JCOMM_H */
