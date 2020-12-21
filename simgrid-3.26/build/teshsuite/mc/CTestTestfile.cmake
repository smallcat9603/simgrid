# CMake generated Testfile for 
# Source directory: /github/simgrid/simgrid-3.26/teshsuite/mc
# Build directory: /github/simgrid/simgrid-3.26/build/teshsuite/mc
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(mc-random-bug-replay "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "platfdir=/github/simgrid/simgrid-3.26/examples/platforms" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/teshsuite/mc/random-bug" "--cd" "/github/simgrid/simgrid-3.26/teshsuite/mc/random-bug" "random-bug-replay.tesh")
set_tests_properties(mc-random-bug-replay PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/teshsuite/mc/CMakeLists.txt;52;ADD_TESH;/github/simgrid/simgrid-3.26/teshsuite/mc/CMakeLists.txt;0;")
