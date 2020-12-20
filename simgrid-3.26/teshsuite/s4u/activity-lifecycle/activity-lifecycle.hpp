/* Copyright (c) 2010-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef ACTIVITY_LIFECYCLE_HPP
#define ACTIVITY_LIFECYCLE_HPP

#include <catch.hpp>

#include <simgrid/s4u.hpp>
#include <xbt/log.h>

#include <vector>

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(s4u_test);

extern std::vector<simgrid::s4u::Host*> all_hosts;

/* Helper function easing the testing of actor's ending condition */
extern void assert_exit(bool exp_success, double duration);

/* Helper function in charge of doing some sanity checks after each test */
extern void assert_cleanup();

/* We need an extra actor here, so that it can sleep until the end of each test */
#define BEGIN_SECTION(descr) SECTION(descr) { simgrid::s4u::Actor::create(descr, all_hosts[0], []()
#define END_SECTION })

#define RUN_SECTION(descr, ...) SECTION(descr) { simgrid::s4u::Actor::create(descr, all_hosts[0], __VA_ARGS__); }

#endif // ACTIVITY_LIFECYCLE_HPP
