# CMake generated Testfile for 
# Source directory: /github/simgrid/simgrid-3.26/tools/graphicator
# Build directory: /github/simgrid/simgrid-3.26/build/tools/graphicator
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(graphicator "/usr/bin/python3.8" "/github/simgrid/simgrid-3.26/build/bin/tesh" "--ignore-jenkins" "--setenv" "srcdir=/github/simgrid/simgrid-3.26" "--setenv" "bindir=/github/simgrid/simgrid-3.26/build/bin" "--cd" "/github/simgrid/simgrid-3.26/build/tools/graphicator" "/github/simgrid/simgrid-3.26/tools/graphicator/graphicator.tesh")
set_tests_properties(graphicator PROPERTIES  _BACKTRACE_TRIPLES "/github/simgrid/simgrid-3.26/tools/cmake/Tests.cmake;49;ADD_TEST;/github/simgrid/simgrid-3.26/tools/graphicator/CMakeLists.txt;5;ADD_TESH;/github/simgrid/simgrid-3.26/tools/graphicator/CMakeLists.txt;0;")
