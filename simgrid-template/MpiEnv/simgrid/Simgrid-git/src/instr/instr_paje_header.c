/* Copyright (c) 2010, 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "instr/instr_private.h"

#ifdef HAVE_TRACING

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(instr_paje_header, instr, "Paje tracing event system (header)");

extern FILE *tracing_file;

static void TRACE_header_PajeDefineContainerType (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeDefineContainerType %d\n", PAJE_DefineContainerType);
  fprintf(tracing_file, "%%       Alias string\n");
  if (basic){
    fprintf(tracing_file, "%%       ContainerType string\n");
  }else{
    fprintf(tracing_file, "%%       Type string\n");
  }
  fprintf(tracing_file, "%%       Name string\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeDefineVariableType (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeDefineVariableType %d\n", PAJE_DefineVariableType);
  fprintf(tracing_file, "%%       Alias string\n");
  if (basic){
    fprintf(tracing_file, "%%       ContainerType string\n");
  }else{
    fprintf(tracing_file, "%%       Type string\n");
  }
  fprintf(tracing_file, "%%       Name string\n");
  fprintf(tracing_file, "%%       Color color\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeDefineStateType (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeDefineStateType %d\n", PAJE_DefineStateType);
  fprintf(tracing_file, "%%       Alias string\n");
  if (basic){
    fprintf(tracing_file, "%%       ContainerType string\n");
  }else{
    fprintf(tracing_file, "%%       Type string\n");
  }
  fprintf(tracing_file, "%%       Name string\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeDefineEventType (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeDefineEventType %d\n", PAJE_DefineEventType);
  fprintf(tracing_file, "%%       Alias string\n");
  if (basic){
    fprintf(tracing_file, "%%       ContainerType string\n");
  }else{
    fprintf(tracing_file, "%%       Type string\n");
  }
  fprintf(tracing_file, "%%       Name string\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeDefineLinkType (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeDefineLinkType %d\n", PAJE_DefineLinkType);
  fprintf(tracing_file, "%%       Alias string\n");
  if (basic){
    fprintf(tracing_file, "%%       ContainerType string\n");
    fprintf(tracing_file, "%%       SourceContainerType string\n");
    fprintf(tracing_file, "%%       DestContainerType string\n");
  }else{
    fprintf(tracing_file, "%%       Type string\n");
    fprintf(tracing_file, "%%       StartContainerType string\n");
    fprintf(tracing_file, "%%       EndContainerType string\n");
  }
  fprintf(tracing_file, "%%       Name string\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeDefineEntityValue (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeDefineEntityValue %d\n", PAJE_DefineEntityValue);
  fprintf(tracing_file, "%%       Alias string\n");
  if (basic){
    fprintf(tracing_file, "%%       EntityType string\n");
  }else{
    fprintf(tracing_file, "%%       Type string\n");
  }
  fprintf(tracing_file, "%%       Name string\n");
  fprintf(tracing_file, "%%       Color color\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeCreateContainer (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeCreateContainer %d\n", PAJE_CreateContainer);
  fprintf(tracing_file, "%%       Time date\n");
  fprintf(tracing_file, "%%       Alias string\n");
  fprintf(tracing_file, "%%       Type string\n");
  fprintf(tracing_file, "%%       Container string\n");
  fprintf(tracing_file, "%%       Name string\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeDestroyContainer (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeDestroyContainer %d\n", PAJE_DestroyContainer);
  fprintf(tracing_file, "%%       Time date\n");
  fprintf(tracing_file, "%%       Type string\n");
  fprintf(tracing_file, "%%       Name string\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeSetVariable (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeSetVariable %d\n", PAJE_SetVariable);
  fprintf(tracing_file, "%%       Time date\n");
  fprintf(tracing_file, "%%       Type string\n");
  fprintf(tracing_file, "%%       Container string\n");
  fprintf(tracing_file, "%%       Value double\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeAddVariable (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeAddVariable %d\n", PAJE_AddVariable);
  fprintf(tracing_file, "%%       Time date\n");
  fprintf(tracing_file, "%%       Type string\n");
  fprintf(tracing_file, "%%       Container string\n");
  fprintf(tracing_file, "%%       Value double\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeSubVariable (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeSubVariable %d\n", PAJE_SubVariable);
  fprintf(tracing_file, "%%       Time date\n");
  fprintf(tracing_file, "%%       Type string\n");
  fprintf(tracing_file, "%%       Container string\n");
  fprintf(tracing_file, "%%       Value double\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}


static void TRACE_header_PajeSetState (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeSetState %d\n", PAJE_SetState);
  fprintf(tracing_file, "%%       Time date\n");
  fprintf(tracing_file, "%%       Type string\n");
  fprintf(tracing_file, "%%       Container string\n");
  fprintf(tracing_file, "%%       Value string\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajePushState (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajePushState %d\n", PAJE_PushState);
  fprintf(tracing_file, "%%       Time date\n");
  fprintf(tracing_file, "%%       Type string\n");
  fprintf(tracing_file, "%%       Container string\n");
  fprintf(tracing_file, "%%       Value string\n");
  if (size) fprintf(tracing_file, "%%       Size int\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajePopState (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajePopState %d\n", PAJE_PopState);
  fprintf(tracing_file, "%%       Time date\n");
  fprintf(tracing_file, "%%       Type string\n");
  fprintf(tracing_file, "%%       Container string\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeResetState (int basic, int size)
{
  if (basic) return;

  fprintf(tracing_file, "%%EventDef PajeResetState %d\n", PAJE_ResetState);
  fprintf(tracing_file, "%%       Time date\n");
  fprintf(tracing_file, "%%       Type string\n");
  fprintf(tracing_file, "%%       Container string\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeStartLink (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeStartLink %d\n", PAJE_StartLink);
  fprintf(tracing_file, "%%       Time date\n");
  fprintf(tracing_file, "%%       Type string\n");
  fprintf(tracing_file, "%%       Container string\n");
  fprintf(tracing_file, "%%       Value string\n");
  if (basic){
    fprintf(tracing_file, "%%       SourceContainer string\n");
  }else{
    fprintf(tracing_file, "%%       StartContainer string\n");
  }
  fprintf(tracing_file, "%%       Key string\n");
  if (size) fprintf(tracing_file, "%%       Size int\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeEndLink (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeEndLink %d\n", PAJE_EndLink);
  fprintf(tracing_file, "%%       Time date\n");
  fprintf(tracing_file, "%%       Type string\n");
  fprintf(tracing_file, "%%       Container string\n");
  fprintf(tracing_file, "%%       Value string\n");
  if (basic){
    fprintf(tracing_file, "%%       DestContainer string\n");
  }else{
    fprintf(tracing_file, "%%       EndContainer string\n");
  }
  fprintf(tracing_file, "%%       Key string\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

static void TRACE_header_PajeNewEvent (int basic, int size)
{
  fprintf(tracing_file, "%%EventDef PajeNewEvent %d\n", PAJE_NewEvent);
  fprintf(tracing_file, "%%       Time date\n");
  fprintf(tracing_file, "%%       Type string\n");
  fprintf(tracing_file, "%%       Container string\n");
  fprintf(tracing_file, "%%       Value string\n");
  fprintf(tracing_file, "%%EndEventDef\n");
}

void TRACE_header(int basic, int size)
{
  XBT_DEBUG ("Define paje header");
  TRACE_header_PajeDefineContainerType (basic, size);
  TRACE_header_PajeDefineVariableType (basic, size);
  TRACE_header_PajeDefineStateType (basic, size);
  TRACE_header_PajeDefineEventType (basic, size);
  TRACE_header_PajeDefineLinkType (basic, size);
  TRACE_header_PajeDefineEntityValue (basic, size);
  TRACE_header_PajeCreateContainer (basic, size);
  TRACE_header_PajeDestroyContainer (basic, size);
  TRACE_header_PajeSetVariable (basic, size);
  TRACE_header_PajeAddVariable (basic, size);
  TRACE_header_PajeSubVariable (basic, size);
  TRACE_header_PajeSetState (basic, size);
  TRACE_header_PajePushState (basic, size);
  TRACE_header_PajePopState (basic, size);
  TRACE_header_PajeResetState (basic, size);
  TRACE_header_PajeStartLink (basic, size);
  TRACE_header_PajeEndLink (basic, size);
  TRACE_header_PajeNewEvent (basic, size);
}

#endif

