/* Copyright (c) 2007-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/actor.h"
#include "simgrid/engine.h"
#include "simgrid/host.h"

#include "xbt/log.h"
#include "xbt/sysdep.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(actor_kill, "Messages specific for this example");

static void victim_on_exit(int ignored1, void* ignored2)
{
  XBT_INFO("I have been killed!");
}

static void victimA_fun(int argc, char* argv[])
{
  sg_actor_on_exit(&victim_on_exit, NULL);
  XBT_INFO("Hello!");
  XBT_INFO("Suspending myself");
  sg_actor_suspend(sg_actor_self()); /* - First suspend itself */
  XBT_INFO("OK, OK. Let's work");    /* - Then is resumed and start to execute a task */
  sg_actor_execute(1e9);
  XBT_INFO("Bye!"); /* - But will never reach the end of it */
}

static void victimB_fun(int argc, char* argv[])
{
  XBT_INFO("Terminate before being killed");
}

static void killer_fun(int argc, char* argv[])
{
  XBT_INFO("Hello!"); /* - First start a victim actor */
  sg_actor_t victimA = sg_actor_create("victim A", sg_host_by_name("Fafard"), victimA_fun, 0, NULL);

  sg_actor_t victimB = sg_actor_create("victim B", sg_host_by_name("Jupiter"), victimB_fun, 0, NULL);
  sg_actor_ref(victimB); // We have to take that ref because victimB will end before we try to kill it

  sg_actor_sleep_for(10.0);

  XBT_INFO("Resume the victim A"); /* - Resume it from its suspended state */
  sg_actor_resume(victimA);
  sg_actor_sleep_for(2.0);

  XBT_INFO("Kill the victim A"); /* - and then kill it */
  sg_actor_kill(victimA);
  sg_actor_sleep_for(1.0);

  XBT_INFO("Kill victimB, even if it's already dead"); /* that's a no-op, there is no zombies in SimGrid */
  sg_actor_kill(victimB); // the actor is automatically garbage-collected after this last reference
  sg_actor_unref(victimB); // Release the ref taken on victimB to avoid to leak memory
  sg_actor_sleep_for(1.0);

  XBT_INFO("Start a new actor, and kill it right away");
  sg_actor_t victimC = sg_actor_create("victim C", sg_host_by_name("Jupiter"), victimA_fun, 0, NULL);
  sg_actor_kill(victimC);
  sg_actor_sleep_for(1.0);

  XBT_INFO("Killing everybody but myself");
  sg_actor_kill_all();

  XBT_INFO("OK, goodbye now. I commit a suicide.");
  sg_actor_exit();

  XBT_INFO("This line will never get displayed: I'm already dead since the previous line.");
}

int main(int argc, char* argv[])
{
  simgrid_init(&argc, argv);
  xbt_assert(argc == 2, "Usage: %s platform_file\n\tExample: %s msg_platform.xml\n", argv[0], argv[0]);

  simgrid_load_platform(argv[1]);

  /* - Create and deploy killer actor, that will create the victim actor  */
  sg_actor_create("killer", sg_host_by_name("Tremblay"), killer_fun, 0, NULL);

  simgrid_run();

  return 0;
}
