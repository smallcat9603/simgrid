/* Copyright (c) 2010-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "activity-lifecycle.hpp"

//========== Creators: create an async activity

// Create a new async execution with given duration
static simgrid::s4u::ActivityPtr create_exec(double duration)
{
  double speed = simgrid::s4u::this_actor::get_host()->get_speed();
  return simgrid::s4u::this_actor::exec_async(speed * duration);
}

// TODO: check other kinds of activities too (Io, Comm, ...)

using creator_type = decltype(create_exec);

//========== Testers: test the completion of an activity

// Calls activity->test() and returns its result
static bool tester_test(const simgrid::s4u::ActivityPtr& activity)
{
  return activity->test();
}

// Calls activity->wait_for(Duration / 128.0) and returns true when activity is terminated, just like test()
template <int Duration> bool tester_wait(const simgrid::s4u::ActivityPtr& activity)
{
  bool ret;
  try {
    activity->wait_for(Duration / 128.0);
    XBT_DEBUG("wait_for() returned normally");
    ret = true;
  } catch (const simgrid::TimeoutException& e) {
    XBT_DEBUG("wait_for() timed out (%s)", e.what());
    ret = false;
  } catch (const simgrid::Exception& e) {
    XBT_DEBUG("wait_for() threw an exception: %s", e.what());
    ret = true;
  }
  return ret;
}

using tester_type = decltype(tester_test);

//========== Waiters: wait for the completion of an activity

// Wait for 6s
static void waiter_sleep6(const simgrid::s4u::ActivityPtr&)
{
  simgrid::s4u::this_actor::sleep_for(6.0);
  XBT_DEBUG("wake up after 6s sleep");
}

// Wait for completion of activity
static void waiter_wait(const simgrid::s4u::ActivityPtr& activity)
{
  activity->wait();
  XBT_DEBUG("end of wait()");
}

using waiter_type = decltype(waiter_wait);

//========== Finally, the test templates

template <creator_type Create, tester_type Test> void test_trivial()
{
  XBT_INFO("Launch an activity for 5s, and let it proceed before test");

  simgrid::s4u::ActorPtr actor = simgrid::s4u::Actor::create("actor", all_hosts[1], []() {
    assert_exit(true, 6.);
    simgrid::s4u::ActivityPtr activity = Create(5.0);
    simgrid::s4u::this_actor::sleep_for(6.0);
    INFO("activity should be terminated now");
    REQUIRE(Test(activity));
  });
  actor->join();
}

template <creator_type Create, tester_type Test> void test_basic()
{
  XBT_INFO("Launch an activity for 5s, and test while it proceeds");

  simgrid::s4u::ActorPtr actor = simgrid::s4u::Actor::create("actor", all_hosts[1], []() {
    assert_exit(true, 6.);
    simgrid::s4u::ActivityPtr activity = Create(5.0);
    for (int i = 0; i < 3; i++) {
      INFO("activity should be still running (i = " << i << ")");
      REQUIRE(not Test(activity));
      simgrid::s4u::this_actor::sleep_for(2.0);
    }
    INFO("activity should be terminated now");
    REQUIRE(Test(activity));
  });
  actor->join();
}

template <creator_type Create, tester_type Test> void test_cancel()
{
  XBT_INFO("Launch an activity for 5s, and cancel it after 2s");

  simgrid::s4u::ActorPtr actor = simgrid::s4u::Actor::create("actor", all_hosts[1], []() {
    assert_exit(true, 2.);
    simgrid::s4u::ActivityPtr activity = Create(5.0);
    simgrid::s4u::this_actor::sleep_for(2.0);
    activity->cancel();
    INFO("activity should be terminated now");
    REQUIRE(Test(activity));
  });
  actor->join();
}

template <creator_type Create, tester_type Test, waiter_type Wait> void test_failure_actor()
{
  XBT_INFO("Launch an activity for 5s, and kill running actor after 2s");

  simgrid::s4u::ActivityPtr activity;
  simgrid::s4u::ActorPtr actor = simgrid::s4u::Actor::create("actor", all_hosts[1], [&activity]() {
    assert_exit(false, 2.);
    activity = Create(5.0);
    Wait(activity);
    FAIL("should not be here!");
  });
  simgrid::s4u::this_actor::sleep_for(2.0);
  INFO("activity should be still running");
  REQUIRE(not Test(activity));
  actor->kill();
  INFO("activity should be terminated now");
  REQUIRE(Test(activity));
}

template <creator_type Create, tester_type Test, waiter_type Wait> void test_failure_host()
{
  XBT_INFO("Launch an activity for 5s, and shutdown host 2s");

  simgrid::s4u::ActivityPtr activity;
  simgrid::s4u::ActorPtr actor = simgrid::s4u::Actor::create("actor", all_hosts[1], [&activity]() {
    assert_exit(false, 2.);
    activity = Create(5.0);
    Wait(activity);
    FAIL("should not be here!");
  });
  simgrid::s4u::this_actor::sleep_for(2.0);
  INFO("activity should be still running");
  REQUIRE(not Test(activity));
  actor->get_host()->turn_off();
  actor->get_host()->turn_on();
  INFO("activity should be terminated now");
  REQUIRE(Test(activity));
}

//==========

TEST_CASE("Activity test/wait: using <tester_test>")
{
  XBT_INFO("#####[ launch next test ]#####");

  RUN_SECTION("exec: run and test once", test_trivial<create_exec, tester_test>);
  RUN_SECTION("exec: run and test many", test_basic<create_exec, tester_test>);
  RUN_SECTION("exec: cancel and test", test_cancel<create_exec, tester_test>);
  RUN_SECTION("exec: actor failure and test / sleep", test_failure_actor<create_exec, tester_test, waiter_sleep6>);
  RUN_SECTION("exec: host failure and test / sleep", test_failure_host<create_exec, tester_test, waiter_sleep6>);
  RUN_SECTION("exec: actor failure and test / wait", test_failure_actor<create_exec, tester_test, waiter_wait>);
  RUN_SECTION("exec: host failure and test / wait", test_failure_host<create_exec, tester_test, waiter_wait>);

  simgrid::s4u::this_actor::sleep_for(10);
  assert_cleanup();
}

TEST_CASE("Activity test/wait: using <tester_wait<0>>")
{
  XBT_INFO("#####[ launch next test ]#####");

  RUN_SECTION("exec: run and wait<0> once", test_trivial<create_exec, tester_wait<0>>);
  // exec: run and wait<0> many
  RUN_SECTION("exec: cancel and wait<0>", test_cancel<create_exec, tester_wait<0>>);
  // exec: actor failure and wait<0> / sleep
  // exec: host failure and wait<0> / sleep
  // exec: actor failure and wait<0> / wait
  // exec: host failure and wait<0> / wait

  simgrid::s4u::this_actor::sleep_for(10);
  assert_cleanup();
}

TEST_CASE("Activity test/wait: using <tester_wait<1>>")
{
  XBT_INFO("#####[ launch next test ]#####");

  RUN_SECTION("exec: run and wait<1> once", test_trivial<create_exec, tester_wait<1>>);
  // exec: run and wait<1> many
  RUN_SECTION("exec: cancel and wait<1>", test_cancel<create_exec, tester_wait<1>>);
  // exec: actor failure and wait<1> / sleep
  // exec: host failure and wait<1> / sleep
  // exec: actor failure and wait<1> / wait
  // exec: host failure and wait<1> / wait

  simgrid::s4u::this_actor::sleep_for(10);
  assert_cleanup();
}

// FIXME: The tests grouped here are currently failing. Once fixed, they should be put in the right section above.
//        The tests can be activated with run-time parameter '*' or, more specifically '[failing]'
TEST_CASE("Activity test/wait: tests currently failing", "[.][failing]")
{
  XBT_INFO("#####[ launch next failing test ]#####");

  // with tester_wait<0>
  RUN_SECTION("exec: run and wait<0> many", test_basic<create_exec, tester_wait<0>>);
  RUN_SECTION("exec: actor failure and wait<0> / sleep", test_failure_actor<create_exec, tester_wait<0>, waiter_sleep6>);
  RUN_SECTION("exec: host failure and wait<0> / sleep", test_failure_host<create_exec, tester_wait<0>, waiter_sleep6>);
  RUN_SECTION("exec: actor failure and wait<0> / wait", test_failure_actor<create_exec, tester_wait<0>, waiter_wait>);
  RUN_SECTION("exec: host failure and wait<0> / wait", test_failure_host<create_exec, tester_wait<0>, waiter_wait>);

  // with tester_test<1>
  RUN_SECTION("exec: run and wait<1> many", test_basic<create_exec, tester_wait<1>>);
  RUN_SECTION("exec: actor failure and wait<1> / sleep", test_failure_actor<create_exec, tester_wait<1>, waiter_sleep6>);
  RUN_SECTION("exec: host failure and wait<1> / sleep", test_failure_host<create_exec, tester_wait<1>, waiter_sleep6>);
  RUN_SECTION("exec: actor failure and wait<1> / wait", test_failure_actor<create_exec, tester_wait<1>, waiter_wait>);
  RUN_SECTION("exec: host failure and wait<1> / wait", test_failure_host<create_exec, tester_wait<1>, waiter_wait>);

  simgrid::s4u::this_actor::sleep_for(10);
  assert_cleanup();
}
