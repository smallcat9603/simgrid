# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.0

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git

# Include any dependencies generated for this target.
include teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/depend.make

# Include the progress variables for this target.
include teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/flags.make

teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o: teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/flags.make
teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o: teshsuite/simdag/network/p2p/test_latency3.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/p2p && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/test_latency3.dir/test_latency3.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/p2p/test_latency3.c

teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test_latency3.dir/test_latency3.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/p2p && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/p2p/test_latency3.c > CMakeFiles/test_latency3.dir/test_latency3.c.i

teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test_latency3.dir/test_latency3.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/p2p && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/p2p/test_latency3.c -o CMakeFiles/test_latency3.dir/test_latency3.c.s

teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o.requires:
.PHONY : teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o.requires

teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o.provides: teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o.requires
	$(MAKE) -f teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/build.make teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o.provides.build
.PHONY : teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o.provides

teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o.provides.build: teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o

# Object files for target test_latency3
test_latency3_OBJECTS = \
"CMakeFiles/test_latency3.dir/test_latency3.c.o"

# External object files for target test_latency3
test_latency3_EXTERNAL_OBJECTS =

teshsuite/simdag/network/p2p/test_latency3: teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o
teshsuite/simdag/network/p2p/test_latency3: teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/build.make
teshsuite/simdag/network/p2p/test_latency3: lib/libsimgrid.so.3.12
teshsuite/simdag/network/p2p/test_latency3: teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable test_latency3"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/p2p && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_latency3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/build: teshsuite/simdag/network/p2p/test_latency3
.PHONY : teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/build

teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/requires: teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/test_latency3.c.o.requires
.PHONY : teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/requires

teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/p2p && $(CMAKE_COMMAND) -P CMakeFiles/test_latency3.dir/cmake_clean.cmake
.PHONY : teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/clean

teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/p2p /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/p2p /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/simdag/network/p2p/CMakeFiles/test_latency3.dir/depend

