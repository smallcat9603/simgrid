# CMake generated Testfile for 
# Source directory: /github/simgrid/simgrid-3.26/examples/smpi/replay_multiple
# Build directory: /github/simgrid/simgrid-3.26/build/examples/smpi/replay_multiple
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(smpi-replay-multiple "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "srcdir=/github/simgrid/simgrid-3.26/examples/smpi/replay_multiple" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/examples/smpi/replay_multiple" "--cd" "/github/simgrid/simgrid-3.26/build/examples/smpi/replay_multiple" "/github/simgrid/simgrid-3.26/examples/smpi/replay_multiple/replay_multiple.tesh")
set_tests_properties(smpi-replay-multiple PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/examples/smpi/replay_multiple/CMakeLists.txt;6;ADD_TESH;/github/simgrid/simgrid-3.26/examples/smpi/replay_multiple/CMakeLists.txt;0;")
