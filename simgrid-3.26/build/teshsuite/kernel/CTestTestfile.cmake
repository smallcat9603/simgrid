# CMake generated Testfile for 
# Source directory: /github/simgrid/simgrid-3.26/teshsuite/kernel
# Build directory: /github/simgrid/simgrid-3.26/build/teshsuite/kernel
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(tesh-kernel-context-default "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/kernel/context-defaults" "--cd" "/github/simgrid/simgrid-3.26/teshsuite/kernel/context-defaults" "factory_ucontext.tesh")
set_tests_properties(tesh-kernel-context-default PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/kernel/CMakeLists.txt;21;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/kernel/CMakeLists.txt;0;")
add_test(tesh-kernel-factory-thread "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:thread" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/kernel/context-defaults" "--cd" "/github/simgrid/simgrid-3.26/teshsuite/kernel/context-defaults" "factory_thread.tesh")
set_tests_properties(tesh-kernel-factory-thread PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/kernel/CMakeLists.txt;32;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/kernel/CMakeLists.txt;0;")
add_test(tesh-kernel-factory-boost "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:boost" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/kernel/context-defaults" "--cd" "/github/simgrid/simgrid-3.26/teshsuite/kernel/context-defaults" "factory_boost.tesh")
set_tests_properties(tesh-kernel-factory-boost PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/kernel/CMakeLists.txt;32;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/kernel/CMakeLists.txt;0;")
add_test(tesh-kernel-factory-ucontext "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--cfg" "contexts/factory:ucontext" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/kernel/context-defaults" "--cd" "/github/simgrid/simgrid-3.26/teshsuite/kernel/context-defaults" "factory_ucontext.tesh")
set_tests_properties(tesh-kernel-factory-ucontext PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/kernel/CMakeLists.txt;32;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/kernel/CMakeLists.txt;0;")
