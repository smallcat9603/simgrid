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
include teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/depend.make

# Include the progress variables for this target.
include teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/flags.make

teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/basic-parsing-test/basic-parsing-test.c.o: teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/flags.make
teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/basic-parsing-test/basic-parsing-test.c.o: ../teshsuite/simdag/basic-parsing-test/basic-parsing-test.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/basic-parsing-test/basic-parsing-test.c.o"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simdag && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/basic-parsing-test.dir/basic-parsing-test/basic-parsing-test.c.o   -c /github/simgrid/simgrid-3.26/teshsuite/simdag/basic-parsing-test/basic-parsing-test.c

teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/basic-parsing-test/basic-parsing-test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/basic-parsing-test.dir/basic-parsing-test/basic-parsing-test.c.i"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simdag && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/teshsuite/simdag/basic-parsing-test/basic-parsing-test.c > CMakeFiles/basic-parsing-test.dir/basic-parsing-test/basic-parsing-test.c.i

teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/basic-parsing-test/basic-parsing-test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/basic-parsing-test.dir/basic-parsing-test/basic-parsing-test.c.s"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simdag && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/teshsuite/simdag/basic-parsing-test/basic-parsing-test.c -o CMakeFiles/basic-parsing-test.dir/basic-parsing-test/basic-parsing-test.c.s

# Object files for target basic-parsing-test
basic__parsing__test_OBJECTS = \
"CMakeFiles/basic-parsing-test.dir/basic-parsing-test/basic-parsing-test.c.o"

# External object files for target basic-parsing-test
basic__parsing__test_EXTERNAL_OBJECTS =

teshsuite/simdag/basic-parsing-test/basic-parsing-test: teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/basic-parsing-test/basic-parsing-test.c.o
teshsuite/simdag/basic-parsing-test/basic-parsing-test: teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/build.make
teshsuite/simdag/basic-parsing-test/basic-parsing-test: lib/libsimgrid.so.3.26
teshsuite/simdag/basic-parsing-test/basic-parsing-test: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
teshsuite/simdag/basic-parsing-test/basic-parsing-test: teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable basic-parsing-test/basic-parsing-test"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simdag && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/basic-parsing-test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/build: teshsuite/simdag/basic-parsing-test/basic-parsing-test

.PHONY : teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/build

teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simdag && $(CMAKE_COMMAND) -P CMakeFiles/basic-parsing-test.dir/cmake_clean.cmake
.PHONY : teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/clean

teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/teshsuite/simdag /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/teshsuite/simdag /github/simgrid/simgrid-3.26/build/teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/simdag/CMakeFiles/basic-parsing-test.dir/depend
