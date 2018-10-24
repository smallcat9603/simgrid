/* file_appender - a dumb log appender which simply prints to a file        */

/* Copyright (c) 2007-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "src/internal_config.h"
#include "src/xbt/log_private.h"
#include "xbt/sysdep.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

static void append_file(xbt_log_appender_t this_, char *str) {
  fputs(str, (FILE *) this_->data);
}

static void free_(xbt_log_appender_t this_) {
  if (this_->data != stderr)
    fclose(this_->data);
}

xbt_log_appender_t xbt_log_appender_file_new(char *arg) {

  xbt_log_appender_t res = xbt_new0(s_xbt_log_appender_t, 1);
  res->do_append         = &append_file;
  res->free_             = &free_;
  if (arg) {
    res->data = (void *) fopen(arg, "w");
    if (res->data == NULL)
      xbt_die("Cannot open file: %s: %s", arg, strerror(errno));
  } else {
    res->data = (void *) stderr;
  }
  return res;
}

struct xbt_log_append2_file_s {
  FILE* file;
  char* filename;
  int count; //negative for roll
  long  int limit;
};
typedef struct xbt_log_append2_file_s* xbt_log_append2_file_t;

#define APPEND2_END_TOKEN             "\n[End of log]\n"
#define APPEND2_END_TOKEN_CLEAR "\n                   "

static void open_append2_file(xbt_log_append2_file_t data){
  if(data->count<0) {
    //Roll
    if (!data->file) {
      data->file= fopen(data->filename, "w");
      if (data->file == NULL)
        xbt_die("Cannot open file: %s: %s", data->filename, strerror(errno));
    } else {
      fputs(APPEND2_END_TOKEN_CLEAR,data->file);
      fseek(data->file,0,SEEK_SET);
    }
  } else{
    //Split
    if(data->file)
      fclose(data->file);
    char newname[512];
    char* pre=xbt_strdup(data->filename);
    char* sep=strchr(pre,'%');
    if(!sep)
      sep=pre+strlen(pre);
    char* post=sep+1;
    *sep='\0';
    snprintf(newname,511,"%s%i%s",pre,data->count,post);
    data->count++;
    data->file= fopen(newname, "w");
    if (data->file == NULL)
      xbt_die("Cannot open file: %s: %s", newname, strerror(errno));
    xbt_free(pre);
  }
}

static void append2_file(xbt_log_appender_t this_, char *str) {
   xbt_log_append2_file_t d=(xbt_log_append2_file_t) this_->data;
   xbt_assert(d->file);
   if(ftell(d->file)>=d->limit) {
     open_append2_file(d);
   }
   fputs(str, d->file);
   if(d->count<0){
     fputs(APPEND2_END_TOKEN,d->file);
     fseek(d->file,-((signed long)strlen(APPEND2_END_TOKEN)),SEEK_CUR);
   }
}

static void free_append2_(xbt_log_appender_t this_)
{
  xbt_log_append2_file_t data = this_->data;
  if (data->file)
    fclose(data->file);
  xbt_free(data->filename);
  xbt_free(data);
}


//syntax is  <maxsize>:<filename>
//If roll is 0, use split files, otherwise, use roll file
//For split, replace %  in the file by the current count
xbt_log_appender_t xbt_log_appender2_file_new(char *arg,int roll) {

  xbt_log_appender_t res      = xbt_new0(s_xbt_log_appender_t, 1);
  res->do_append              = &append2_file;
  res->free_                  = &free_append2_;
  xbt_log_append2_file_t data = xbt_new0(struct xbt_log_append2_file_s, 1);
  xbt_assert(arg);
  char* buf=xbt_strdup(arg);
  char* sep=strchr(buf,':');
  xbt_assert(sep>0);
  data->filename=xbt_strdup(sep+1);
  *sep='\0';
  char *endptr;
  data->limit=strtol(buf,&endptr,10);
  xbt_assert(endptr[0]=='\0', "Invalid buffer size: %s", buf);
  xbt_free(buf);
  if(roll)
    data->count=-1;
  else
    data->count=0;
  open_append2_file(data);
  res->data = data;
  return res;
}
