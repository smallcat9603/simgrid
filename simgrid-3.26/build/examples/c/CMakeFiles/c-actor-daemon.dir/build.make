# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_SOURCE_DIR = /github/simgrid/simgrid-3.26

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /github/simgrid/simgrid-3.26/build

# Include any dependencies generated for this target.
include examples/c/CMakeFiles/c-actor-daemon.dir/depend.make

# Include the progress variables for this target.
include examples/c/CMakeFiles/c-actor-daemon.dir/progress.make

# Include the compile flags for this target's objects.
include examples/c/CMakeFiles/c-actor-daemon.dir/flags.make

examples/c/CMakeFiles/c-actor-daemon.dir/actor-daemon/actor-daemon.c.o: examples/c/CMakeFiles/c-actor-daemon.dir/flags.make
examples/c/CMakeFiles/c-actor-daemon.dir/actor-daemon/actor-daemon.c.o: ../examples/c/actor-daemon/actor-daemon.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object examples/c/CMakeFiles/c-actor-daemon.dir/actor-daemon/actor-daemon.c.o"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/c-actor-daemon.dir/actor-daemon/actor-daemon.c.o   -c /github/simgrid/simgrid-3.26/examples/c/actor-daemon/actor-daemon.c

examples/c/CMakeFiles/c-actor-daemon.dir/actor-daemon/actor-daemon.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/c-actor-daemon.dir/actor-daemon/actor-daemon.c.i"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/examples/c/actor-daemon/actor-daemon.c > CMakeFiles/c-actor-daemon.dir/actor-daemon/actor-daemon.c.i

examples/c/CMakeFiles/c-actor-daemon.dir/actor-daemon/actor-daemon.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/c-actor-daemon.dir/actor-daemon/actor-daemon.c.s"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/examples/c/actor-daemon/actor-daemon.c -o CMakeFiles/c-actor-daemon.dir/actor-daemon/actor-daemon.c.s

# Object files for target c-actor-daemon
c__actor__daemon_OBJECTS = \
"CMakeFiles/c-actor-daemon.dir/actor-daemon/actor-daemon.c.o"

# External object files for target c-actor-daemon
c__actor__daemon_EXTERNAL_OBJECTS =

examples/c/actor-daemon/c-actor-daemon: examples/c/CMakeFiles/c-actor-daemon.dir/actor-daemon/actor-daemon.c.o
examples/c/actor-daemon/c-actor-daemon: examples/c/CMakeFiles/c-actor-daemon.dir/build.make
examples/c/actor-daemon/c-actor-daemon: lib/libsimgrid.so.3.26
examples/c/actor-daemon/c-actor-daemon: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
examples/c/actor-daemon/c-actor-daemon: examples/c/CMakeFiles/c-actor-daemon.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable actor-daemon/c-actor-daemon"
	cd /github/simgrid/simgrid-3.26/build/examples/c && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/c-actor-daemon.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/c/CMakeFiles/c-actor-daemon.dir/build: examples/c/actor-daemon/c-actor-daemon

.PHONY : examples/c/CMakeFiles/c-actor-daemon.dir/build

examples/c/CMakeFiles/c-actor-daemon.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/examples/c && $(CMAKE_COMMAND) -P CMakeFiles/c-actor-daemon.dir/cmake_clean.cmake
.PHONY : examples/c/CMakeFiles/c-actor-daemon.dir/clean

examples/c/CMakeFiles/c-actor-daemon.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/examples/c /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/examples/c /github/simgrid/simgrid-3.26/build/examples/c/CMakeFiles/c-actor-daemon.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/c/CMakeFiles/c-actor-daemon.dir/depend

