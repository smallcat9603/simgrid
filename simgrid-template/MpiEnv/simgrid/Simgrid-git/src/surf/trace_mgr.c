/* Copyright (c) 2004-2005, 2007, 2009-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt/sysdep.h"
#include "xbt/log.h"
#include "xbt/str.h"
#include "xbt/dict.h"
#include "trace_mgr_private.h"
#include "surf_private.h"
#include "xbt/RngStream.h"
#include <math.h>

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(surf_trace, surf, "Surf trace management");

static xbt_dict_t trace_list = NULL;

XBT_INLINE tmgr_history_t tmgr_history_new(void)
{
  tmgr_history_t h;

  h = xbt_new0(s_tmgr_history_t, 1);

  h->heap = xbt_heap_new(8, xbt_free_f);        /* Why 8 ? Well, why not... */

  return h;
}

XBT_INLINE void tmgr_history_free(tmgr_history_t h)
{
  xbt_heap_free(h->heap);
  free(h);
}


/**
 * \brief Create a #tmgr_trace_t from probabilist generators
 *
 * This trace will generate an infinite set of events.
 * It needs two #probabilist_event_generator_t. The date when the event are
 * triggered is directed by date_generator, and will be interpreted as seconds.
 * The value of the event is set by value_generator. The value should be between
 * 0 and 1.
 *
 * \param id The name of the trace
 * \param date_generator The #probabilist_event_generator_t which generates the time
 *        between two events
 * \param generator2 The #probabilist_event_generator_t which generates the value
 *        of each events.
 * \return The new #tmgr_trace_t
 */
tmgr_trace_t tmgr_trace_generator_value(const char *id,
                                  probabilist_event_generator_t date_generator,
                                  probabilist_event_generator_t value_generator)
{
  tmgr_trace_t trace = NULL;

  trace = xbt_new0(s_tmgr_trace_t, 1);
  trace->type = e_trace_probabilist;

  trace->s_probabilist.event_generator[0] = date_generator;
  trace->s_probabilist.event_generator[1] = value_generator;
  trace->s_probabilist.is_state_trace = 0;

  return trace;
}

/**
 * \brief Create a #tmgr_trace_t from probabilist generators
 *
 * This trace will generate an infinite set of events. Value of the events
 * will be alternatively 0 and 1, so this should be used as a state trace.
 *
 * \param id The name of the trace
 * \param date_generator The #probabilist_event_generator_t which generates the time
 *        between two events
 * \param first_event_value Set the first event value
 * \return The new #tmgr_trace_t
 */
tmgr_trace_t tmgr_trace_generator_state(const char *id,
                                  probabilist_event_generator_t date_generator,
                                  e_surf_resource_state_t first_event_value)
{
  tmgr_trace_t trace = NULL;

  trace = xbt_new0(s_tmgr_trace_t, 1);
  trace->type = e_trace_probabilist;

  trace->s_probabilist.event_generator[0] = date_generator;
  trace->s_probabilist.event_generator[1] = date_generator;
  trace->s_probabilist.is_state_trace = 1;
  trace->s_probabilist.next_event = (first_event_value==SURF_RESOURCE_ON ? 1 : 0);

  return trace;
}

/**
 * \brief Create a #tmgr_trace_t from probabilist generators
 *
 * This trace will generate an infinite set of events. Value of the events
 * will be alternatively 0 and 1, so this should be used as a state trace.
 *
 * \param id The name of the trace
 * \param avail_duration_generator The #probabilist_event_generator_t which
 *        set the duration of the available state, (ie 1 value)
 * \param unavail_duration_generator The #probabilist_event_generator_t which
 *        set the duration of the unavailable state, (ie 0 value)
 * \param first_event_value Set the first event value
 * \return The new #tmgr_trace_t
 */
tmgr_trace_t tmgr_trace_generator_avail_unavail(const char *id,
                                probabilist_event_generator_t avail_duration_generator,
                                probabilist_event_generator_t unavail_duration_generator,
                                e_surf_resource_state_t first_event_value)
{
  tmgr_trace_t trace = NULL;

  trace = xbt_new0(s_tmgr_trace_t, 1);
  trace->type = e_trace_probabilist;

  trace->s_probabilist.event_generator[0] = unavail_duration_generator;
  trace->s_probabilist.event_generator[1] = avail_duration_generator;
  trace->s_probabilist.is_state_trace = 1;
  trace->s_probabilist.next_event = (first_event_value==SURF_RESOURCE_ON ? 1 : 0);

  return trace;
}

/**
 * \brief Create a new #probabilist_event_generator_t following the uniform distribution
 *
 * This generator will generate uniformly distributed random values between min and max
 * The id is important : it controls the seed of the generator. So, generators with the
 * same id and the same parameters will generate the same values.
 *
 * \param id The name of the generator
 * \param min The minimal generated value
 * \param max The maximal generated value
 * \return a new #probabilist_event_generator_t
 */
probabilist_event_generator_t tmgr_event_generator_new_uniform(const char* id,
                                                               double min,
                                                               double max)
{
  probabilist_event_generator_t event_generator = NULL;
  RngStream rng_stream = NULL;

  rng_stream = sg_platf_rng_stream_get(id);

  event_generator = xbt_new0(s_probabilist_event_generator_t, 1);
  event_generator->type = e_generator_uniform;
  event_generator->s_uniform_parameters.min = min;
  event_generator->s_uniform_parameters.max = max;
  event_generator->rng_stream = rng_stream;

  tmgr_event_generator_next_value(event_generator);

  return event_generator;
}


/**
 * \brief Create a new #probabilist_event_generator_t following the exponential distribution
 *
 * This generator will generate random values following the exponential distribution.
 * The mean value is 1/rate .
 * The id is important : it controls the seed of the generator. So, generators with the
 * same id and the same parameters will generate the same values.
 *
 * \param id The name of the generator
 * \param rate The rate parameter
 * \return a new #probabilist_event_generator_t
 */
probabilist_event_generator_t tmgr_event_generator_new_exponential(const char* id,
                                                                   double rate)
{
  probabilist_event_generator_t event_generator = NULL;
  RngStream rng_stream = NULL;

  rng_stream = sg_platf_rng_stream_get(id);

  event_generator = xbt_new0(s_probabilist_event_generator_t, 1);
  event_generator->type = e_generator_exponential;
  event_generator->s_exponential_parameters.rate = rate;
  event_generator->rng_stream = rng_stream;

  tmgr_event_generator_next_value(event_generator);

  return event_generator;
}

/**
 * \brief Create a new #probabilist_event_generator_t following the weibull distribution
 *
 * This generator will generate random values following the weibull distribution.
 * The id is important : it controls the seed of the generator. So, generators with the
 * same id and the same parameters will generate the same values.
 *
 * \param id The name of the generator
 * \param scale The scale parameter
 * \param shape The shape parameter
 * \return a new #probabilist_event_generator_t
 */
probabilist_event_generator_t tmgr_event_generator_new_weibull(const char* id,
                                                               double scale,
                                                               double shape)
{
  probabilist_event_generator_t event_generator = NULL;
  RngStream rng_stream = NULL;

  rng_stream = sg_platf_rng_stream_get(id);

  event_generator = xbt_new0(s_probabilist_event_generator_t, 1);
  event_generator->type = e_generator_weibull;
  event_generator->s_weibull_parameters.scale = scale;
  event_generator->s_weibull_parameters.shape = shape;
  event_generator->rng_stream = rng_stream;

  tmgr_event_generator_next_value(event_generator);

  return event_generator;
}
/**
 * \brief Get the next random value of a #probabilist_event_generator_t
 * \param generator The #probabilist_event_generator_t
 * \return the next random value
 */
double tmgr_event_generator_next_value(probabilist_event_generator_t generator)
{

  switch(generator->type) {
    case e_generator_uniform:
      generator->next_value = (RngStream_RandU01(generator->rng_stream)
                  * (generator->s_uniform_parameters.max - generator->s_uniform_parameters.min))
                  + generator->s_uniform_parameters.min;
      break;
    case e_generator_exponential:
      generator->next_value = -log(RngStream_RandU01(generator->rng_stream))
                              / generator->s_exponential_parameters.rate;
      break;
    case e_generator_weibull:
      generator->next_value = generator->s_weibull_parameters.scale
                              * pow( -log(RngStream_RandU01(generator->rng_stream)),
                                    1.0 / generator->s_weibull_parameters.shape );
  }

  return generator->next_value;
}

tmgr_trace_t tmgr_trace_new_from_string(const char *id, const char *input,
                                        double periodicity)
{
  tmgr_trace_t trace = NULL;
  int linecount = 0;
  s_tmgr_event_t event;
  tmgr_event_t last_event = NULL;
  xbt_dynar_t list;
  unsigned int cpt;
  char *val;

  if (trace_list) {
    trace = xbt_dict_get_or_null(trace_list, id);
    if (trace) {
      XBT_WARN("Ignoring redefinition of trace %s", id);
      return trace;
    }
  }

  xbt_assert(periodicity >= 0,
              "Invalid periodicity %g (must be positive)", periodicity);

  trace = xbt_new0(s_tmgr_trace_t, 1);
  trace->type = e_trace_list;
  trace->s_list.event_list = xbt_dynar_new(sizeof(s_tmgr_event_t), NULL);

  list = xbt_str_split(input, "\n\r");

  xbt_dynar_foreach(list, cpt, val) {
    linecount++;
    xbt_str_trim(val, " \t\n\r\x0B");
    if (val[0] == '#' || val[0] == '\0' || val[0] == '%')
      continue;

    if (sscanf(val, "PERIODICITY " "%lg" "\n", &periodicity) == 1)
      continue;

    if (sscanf(val, "%lg" " " "%lg" "\n", &event.delta, &event.value) != 2)
      xbt_die("%s:%d: Syntax error in trace\n%s", id, linecount, input);

    if (last_event) {
      if (last_event->delta > event.delta) {
        xbt_die("%s:%d: Invalid trace: Events must be sorted, "
                "but time %g > time %g.\n%s",
                id, linecount, last_event->delta, event.delta, input);
      }
      last_event->delta = event.delta - last_event->delta;
    } else {
      if(event.delta > 0.0){
        s_tmgr_event_t first_event;
        first_event.delta=event.delta;
        first_event.value=-1.0;
        xbt_dynar_push(trace->s_list.event_list, &first_event);
      }
    }
    xbt_dynar_push(trace->s_list.event_list, &event);
    last_event =
        xbt_dynar_get_ptr(trace->s_list.event_list,
                          xbt_dynar_length(trace->s_list.event_list) - 1);
  }
  if (last_event)
    last_event->delta = periodicity;

  if (!trace_list)
    trace_list = xbt_dict_new_homogeneous((void (*)(void *)) tmgr_trace_free);

  xbt_dict_set(trace_list, id, (void *) trace, NULL);

  xbt_dynar_free(&list);
  return trace;
}

tmgr_trace_t tmgr_trace_new_from_file(const char *filename)
{
  char *tstr = NULL;
  FILE *f = NULL;
  tmgr_trace_t trace = NULL;

  if ((!filename) || (strcmp(filename, "") == 0))
    return NULL;

  if (trace_list) {
    trace = xbt_dict_get_or_null(trace_list, filename);
    if (trace) {
      XBT_WARN("Ignoring redefinition of trace %s", filename);
      return trace;
    }
  }

  f = surf_fopen(filename, "r");
  if (f == NULL)
    xbt_die("Cannot open file '%s' (path=%s)", filename,
            xbt_str_join(surf_path, ":"));

  tstr = xbt_str_from_file(f);
  fclose(f);
  trace = tmgr_trace_new_from_string(filename, tstr, 0.);
  xbt_free(tstr);

  return trace;
}

tmgr_trace_t tmgr_empty_trace_new(void)
{
  tmgr_trace_t trace = NULL;
  s_tmgr_event_t event;

  trace = xbt_new0(s_tmgr_trace_t, 1);
  trace->type = e_trace_list;
  trace->s_list.event_list = xbt_dynar_new(sizeof(s_tmgr_event_t), NULL);

  event.delta = 0.0;
  event.value = 0.0;
  xbt_dynar_push(trace->s_list.event_list, &event);

  return trace;
}

XBT_INLINE void tmgr_trace_free(tmgr_trace_t trace)
{
  if (!trace)
    return;

  switch(trace->type) {
    case e_trace_list:
      xbt_dynar_free(&(trace->s_list.event_list));
      break;
    case e_trace_probabilist:
      THROW_UNIMPLEMENTED;
      break;
  }
  free(trace);
}

tmgr_trace_event_t tmgr_history_add_trace(tmgr_history_t h,
                                          tmgr_trace_t trace,
                                          double start_time,
                                          unsigned int offset, void *model)
{
  tmgr_trace_event_t trace_event = NULL;

  trace_event = xbt_new0(s_tmgr_trace_event_t, 1);
  trace_event->trace = trace;
  trace_event->idx = offset;
  trace_event->model = model;

  if(trace->type == e_trace_list) {
    xbt_assert((trace_event->idx < xbt_dynar_length(trace->s_list.event_list)),
              "You're referring to an event that does not exist!");
  }

  xbt_heap_push(h->heap, trace_event, start_time);

  return trace_event;
}

XBT_INLINE double tmgr_history_next_date(tmgr_history_t h)
{
  if (xbt_heap_size(h->heap))
    return (xbt_heap_maxkey(h->heap));
  else
    return -1.0;
}

tmgr_trace_event_t tmgr_history_get_next_event_leq(tmgr_history_t h,
                                                   double date,
                                                   double *value,
                                                   void **model)
{
  double event_date = tmgr_history_next_date(h);
  tmgr_trace_event_t trace_event = NULL;
  tmgr_event_t event = NULL;
  tmgr_trace_t trace = NULL;
  double event_delta;

  if (event_date > date)
    return NULL;

  if (!(trace_event = xbt_heap_pop(h->heap)))
    return NULL;

  trace = trace_event->trace;
  *model = trace_event->model;

  switch(trace->type) {
    case e_trace_list:

      event = xbt_dynar_get_ptr(trace->s_list.event_list, trace_event->idx);

      *value = event->value;

      if (trace_event->idx < xbt_dynar_length(trace->s_list.event_list) - 1) {
        xbt_heap_push(h->heap, trace_event, event_date + event->delta);
        trace_event->idx++;
      } else if (event->delta > 0) {        /* Last element, checking for periodicity */
        xbt_heap_push(h->heap, trace_event, event_date + event->delta);
        trace_event->idx = 1; /* not 0 as the first event is a placeholder to handle when events really start */
      } else {                      /* We don't need this trace_event anymore */
        trace_event->free_me = 1;
      }
      break;

    case e_trace_probabilist:

      //FIXME : not tested yet
      if(trace->s_probabilist.is_state_trace) {
        *value = (double) trace->s_probabilist.next_event;
        if(trace->s_probabilist.next_event == 0) {
          event_delta = tmgr_event_generator_next_value(trace->s_probabilist.event_generator[0]);
          trace->s_probabilist.next_event = 1;
        } else {
          event_delta = tmgr_event_generator_next_value(trace->s_probabilist.event_generator[1]);
          trace->s_probabilist.next_event = 0;
        }
      } else {
        event_delta = tmgr_event_generator_next_value(trace->s_probabilist.event_generator[0]);
        *value = tmgr_event_generator_next_value(trace->s_probabilist.event_generator[1]);
      }
      xbt_heap_push(h->heap, trace_event, event_date + event_delta);
      XBT_DEBUG("Generating a new event at date %f, with value %f", event_date + event_delta, *value);

      break;
  }

  return trace_event;
}

XBT_INLINE void tmgr_finalize(void)
{
  xbt_dict_free(&trace_list);
}

int tmgr_trace_event_free(tmgr_trace_event_t trace_event)
{
  if (trace_event->free_me) {
    xbt_free(trace_event);
    return 1;
  }
  return 0;
}
