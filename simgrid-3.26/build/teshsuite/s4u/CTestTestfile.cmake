# CMake generated Testfile for 
# Source directory: /github/simgrid/simgrid-3.26/teshsuite/s4u
# Build directory: /github/simgrid/simgrid-3.26/build/teshsuite/s4u
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(tesh-s4u-actor-thread "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:thread" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/actor" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor" "/github/simgrid/simgrid-3.26/teshsuite/s4u/actor/actor.tesh")
set_tests_properties(tesh-s4u-actor-thread PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-actor-ucontext "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:ucontext" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/actor" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor" "/github/simgrid/simgrid-3.26/teshsuite/s4u/actor/actor.tesh")
set_tests_properties(tesh-s4u-actor-ucontext PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-actor-boost "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:boost" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/actor" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor" "/github/simgrid/simgrid-3.26/teshsuite/s4u/actor/actor.tesh")
set_tests_properties(tesh-s4u-actor-boost PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-actor-autorestart-thread "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:thread" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor-autorestart" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/actor-autorestart" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor-autorestart" "/github/simgrid/simgrid-3.26/teshsuite/s4u/actor-autorestart/actor-autorestart.tesh")
set_tests_properties(tesh-s4u-actor-autorestart-thread PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-actor-autorestart-ucontext "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:ucontext" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor-autorestart" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/actor-autorestart" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor-autorestart" "/github/simgrid/simgrid-3.26/teshsuite/s4u/actor-autorestart/actor-autorestart.tesh")
set_tests_properties(tesh-s4u-actor-autorestart-ucontext PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-actor-autorestart-boost "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:boost" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor-autorestart" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/actor-autorestart" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor-autorestart" "/github/simgrid/simgrid-3.26/teshsuite/s4u/actor-autorestart/actor-autorestart.tesh")
set_tests_properties(tesh-s4u-actor-autorestart-boost PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-actor-suspend-thread "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:thread" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor-suspend" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/actor-suspend" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor-suspend" "/github/simgrid/simgrid-3.26/teshsuite/s4u/actor-suspend/actor-suspend.tesh")
set_tests_properties(tesh-s4u-actor-suspend-thread PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-actor-suspend-ucontext "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:ucontext" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor-suspend" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/actor-suspend" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor-suspend" "/github/simgrid/simgrid-3.26/teshsuite/s4u/actor-suspend/actor-suspend.tesh")
set_tests_properties(tesh-s4u-actor-suspend-ucontext PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-actor-suspend-boost "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:boost" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor-suspend" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/actor-suspend" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/actor-suspend" "/github/simgrid/simgrid-3.26/teshsuite/s4u/actor-suspend/actor-suspend.tesh")
set_tests_properties(tesh-s4u-actor-suspend-boost PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-activity-lifecycle-thread "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:thread" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/activity-lifecycle" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/activity-lifecycle" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/activity-lifecycle" "/github/simgrid/simgrid-3.26/teshsuite/s4u/activity-lifecycle/activity-lifecycle.tesh")
set_tests_properties(tesh-s4u-activity-lifecycle-thread PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-activity-lifecycle-ucontext "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:ucontext" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/activity-lifecycle" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/activity-lifecycle" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/activity-lifecycle" "/github/simgrid/simgrid-3.26/teshsuite/s4u/activity-lifecycle/activity-lifecycle.tesh")
set_tests_properties(tesh-s4u-activity-lifecycle-ucontext PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-activity-lifecycle-boost "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:boost" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/activity-lifecycle" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/activity-lifecycle" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/activity-lifecycle" "/github/simgrid/simgrid-3.26/teshsuite/s4u/activity-lifecycle/activity-lifecycle.tesh")
set_tests_properties(tesh-s4u-activity-lifecycle-boost PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-comm-get-sender-thread "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:thread" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/comm-get-sender" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/comm-get-sender" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/comm-get-sender" "/github/simgrid/simgrid-3.26/teshsuite/s4u/comm-get-sender/comm-get-sender.tesh")
set_tests_properties(tesh-s4u-comm-get-sender-thread PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-comm-get-sender-ucontext "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:ucontext" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/comm-get-sender" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/comm-get-sender" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/comm-get-sender" "/github/simgrid/simgrid-3.26/teshsuite/s4u/comm-get-sender/comm-get-sender.tesh")
set_tests_properties(tesh-s4u-comm-get-sender-ucontext PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-comm-get-sender-boost "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:boost" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/comm-get-sender" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/comm-get-sender" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/comm-get-sender" "/github/simgrid/simgrid-3.26/teshsuite/s4u/comm-get-sender/comm-get-sender.tesh")
set_tests_properties(tesh-s4u-comm-get-sender-boost PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-wait-any-for-thread "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:thread" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/wait-any-for" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/wait-any-for" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/wait-any-for" "/github/simgrid/simgrid-3.26/teshsuite/s4u/wait-any-for/wait-any-for.tesh")
set_tests_properties(tesh-s4u-wait-any-for-thread PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-wait-any-for-ucontext "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:ucontext" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/wait-any-for" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/wait-any-for" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/wait-any-for" "/github/simgrid/simgrid-3.26/teshsuite/s4u/wait-any-for/wait-any-for.tesh")
set_tests_properties(tesh-s4u-wait-any-for-ucontext PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-wait-any-for-boost "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:boost" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/wait-any-for" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/wait-any-for" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/wait-any-for" "/github/simgrid/simgrid-3.26/teshsuite/s4u/wait-any-for/wait-any-for.tesh")
set_tests_properties(tesh-s4u-wait-any-for-boost PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-cloud-interrupt-migration-thread "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:thread" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-interrupt-migration" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-interrupt-migration" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-interrupt-migration" "/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-interrupt-migration/cloud-interrupt-migration.tesh")
set_tests_properties(tesh-s4u-cloud-interrupt-migration-thread PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-cloud-interrupt-migration-ucontext "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:ucontext" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-interrupt-migration" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-interrupt-migration" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-interrupt-migration" "/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-interrupt-migration/cloud-interrupt-migration.tesh")
set_tests_properties(tesh-s4u-cloud-interrupt-migration-ucontext PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-cloud-interrupt-migration-boost "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:boost" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-interrupt-migration" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-interrupt-migration" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-interrupt-migration" "/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-interrupt-migration/cloud-interrupt-migration.tesh")
set_tests_properties(tesh-s4u-cloud-interrupt-migration-boost PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-cloud-two-execs-thread "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:thread" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-two-execs" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-two-execs" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-two-execs" "/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-two-execs/cloud-two-execs.tesh")
set_tests_properties(tesh-s4u-cloud-two-execs-thread PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-cloud-two-execs-ucontext "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:ucontext" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-two-execs" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-two-execs" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-two-execs" "/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-two-execs/cloud-two-execs.tesh")
set_tests_properties(tesh-s4u-cloud-two-execs-ucontext PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-cloud-two-execs-boost "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:boost" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-two-execs" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-two-execs" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-two-execs" "/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-two-execs/cloud-two-execs.tesh")
set_tests_properties(tesh-s4u-cloud-two-execs-boost PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-concurrent_rw-thread "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:thread" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/concurrent_rw" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/concurrent_rw" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/concurrent_rw" "/github/simgrid/simgrid-3.26/teshsuite/s4u/concurrent_rw/concurrent_rw.tesh")
set_tests_properties(tesh-s4u-concurrent_rw-thread PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-concurrent_rw-ucontext "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:ucontext" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/concurrent_rw" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/concurrent_rw" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/concurrent_rw" "/github/simgrid/simgrid-3.26/teshsuite/s4u/concurrent_rw/concurrent_rw.tesh")
set_tests_properties(tesh-s4u-concurrent_rw-ucontext PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-concurrent_rw-boost "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:boost" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/concurrent_rw" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/concurrent_rw" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/concurrent_rw" "/github/simgrid/simgrid-3.26/teshsuite/s4u/concurrent_rw/concurrent_rw.tesh")
set_tests_properties(tesh-s4u-concurrent_rw-boost PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-vm-live-migration-thread "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:thread" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/vm-live-migration" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/vm-live-migration" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/vm-live-migration" "/github/simgrid/simgrid-3.26/teshsuite/s4u/vm-live-migration/vm-live-migration.tesh")
set_tests_properties(tesh-s4u-vm-live-migration-thread PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-vm-live-migration-ucontext "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:ucontext" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/vm-live-migration" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/vm-live-migration" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/vm-live-migration" "/github/simgrid/simgrid-3.26/teshsuite/s4u/vm-live-migration/vm-live-migration.tesh")
set_tests_properties(tesh-s4u-vm-live-migration-ucontext PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-vm-live-migration-boost "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:boost" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/s4u/vm-live-migration" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/vm-live-migration" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/vm-live-migration" "/github/simgrid/simgrid-3.26/teshsuite/s4u/vm-live-migration/vm-live-migration.tesh")
set_tests_properties(tesh-s4u-vm-live-migration-boost PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;64;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;33;ADD_TESH_FACTORIES;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-basic-link-test "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/basic-link-test" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/basic-link-test" "/github/simgrid/simgrid-3.26/teshsuite/s4u/basic-link-test/basic-link-test.tesh")
set_tests_properties(tesh-s4u-basic-link-test PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;38;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-host-on-off "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/host-on-off" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/host-on-off" "/github/simgrid/simgrid-3.26/teshsuite/s4u/host-on-off/host-on-off.tesh")
set_tests_properties(tesh-s4u-host-on-off PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;38;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-host-on-off-actors "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/host-on-off-actors" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/host-on-off-actors" "/github/simgrid/simgrid-3.26/teshsuite/s4u/host-on-off-actors/host-on-off-actors.tesh")
set_tests_properties(tesh-s4u-host-on-off-actors PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;38;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-host-on-off-recv "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/host-on-off-recv" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/host-on-off-recv" "/github/simgrid/simgrid-3.26/teshsuite/s4u/host-on-off-recv/host-on-off-recv.tesh")
set_tests_properties(tesh-s4u-host-on-off-recv PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;38;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-is-router "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/is-router" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/is-router" "/github/simgrid/simgrid-3.26/teshsuite/s4u/is-router/is-router.tesh")
set_tests_properties(tesh-s4u-is-router PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;38;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-listen_async "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/listen_async" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/listen_async" "/github/simgrid/simgrid-3.26/teshsuite/s4u/listen_async/listen_async.tesh")
set_tests_properties(tesh-s4u-listen_async PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;38;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-pid "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/pid" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/pid" "/github/simgrid/simgrid-3.26/teshsuite/s4u/pid/pid.tesh")
set_tests_properties(tesh-s4u-pid PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;38;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-storage_client_server "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/storage_client_server" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/storage_client_server" "/github/simgrid/simgrid-3.26/teshsuite/s4u/storage_client_server/storage_client_server.tesh")
set_tests_properties(tesh-s4u-storage_client_server PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;38;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-cloud-sharing "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-sharing" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cloud-sharing" "/github/simgrid/simgrid-3.26/teshsuite/s4u/cloud-sharing/cloud-sharing.tesh")
set_tests_properties(tesh-s4u-cloud-sharing PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;38;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-trace-integration "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/teshsuite/s4u/trace-integration" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--cd" "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/trace-integration" "/github/simgrid/simgrid-3.26/teshsuite/s4u/trace-integration/trace-integration.tesh")
set_tests_properties(tesh-s4u-trace-integration PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;38;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")
add_test(tesh-s4u-comm-pt2pt "/github/simgrid/simgrid-3.26/build/teshsuite/s4u/comm-pt2pt/comm-pt2pt" "/github/simgrid/simgrid-3.26/examples/platforms/cluster_backbone.xml")
set_tests_properties(tesh-s4u-comm-pt2pt PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;42;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/s4u/CMakeLists.txt;0;")