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
include examples/c/CMakeFiles/c-cloud-capping.dir/depend.make

# Include the progress variables for this target.
include examples/c/CMakeFiles/c-cloud-capping.dir/progress.make

# Include the compile flags for this target's objects.
include examples/c/CMakeFiles/c-cloud-capping.dir/flags.make

examples/c/CMakeFiles/c-cloud-capping.dir/cloud-capping/cloud-capping.c.o: examples/c/CMakeFiles/c-cloud-capping.dir/flags.make
examples/c/CMakeFiles/c-cloud-capping.dir/cloud-capping/cloud-capping.c.o: ../examples/c/cloud-capping/cloud-capping.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object examples/c/CMakeFiles/c-cloud-capping.dir/cloud-capping/cloud-capping.c.o"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/c-cloud-capping.dir/cloud-capping/cloud-capping.c.o   -c /github/simgrid/simgrid-3.26/examples/c/cloud-capping/cloud-capping.c

examples/c/CMakeFiles/c-cloud-capping.dir/cloud-capping/cloud-capping.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/c-cloud-capping.dir/cloud-capping/cloud-capping.c.i"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/examples/c/cloud-capping/cloud-capping.c > CMakeFiles/c-cloud-capping.dir/cloud-capping/cloud-capping.c.i

examples/c/CMakeFiles/c-cloud-capping.dir/cloud-capping/cloud-capping.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/c-cloud-capping.dir/cloud-capping/cloud-capping.c.s"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/examples/c/cloud-capping/cloud-capping.c -o CMakeFiles/c-cloud-capping.dir/cloud-capping/cloud-capping.c.s

# Object files for target c-cloud-capping
c__cloud__capping_OBJECTS = \
"CMakeFiles/c-cloud-capping.dir/cloud-capping/cloud-capping.c.o"

# External object files for target c-cloud-capping
c__cloud__capping_EXTERNAL_OBJECTS =

examples/c/cloud-capping/c-cloud-capping: examples/c/CMakeFiles/c-cloud-capping.dir/cloud-capping/cloud-capping.c.o
examples/c/cloud-capping/c-cloud-capping: examples/c/CMakeFiles/c-cloud-capping.dir/build.make
examples/c/cloud-capping/c-cloud-capping: lib/libsimgrid.so.3.26
examples/c/cloud-capping/c-cloud-capping: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
examples/c/cloud-capping/c-cloud-capping: examples/c/CMakeFiles/c-cloud-capping.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable cloud-capping/c-cloud-capping"
	cd /github/simgrid/simgrid-3.26/build/examples/c && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/c-cloud-capping.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/c/CMakeFiles/c-cloud-capping.dir/build: examples/c/cloud-capping/c-cloud-capping

.PHONY : examples/c/CMakeFiles/c-cloud-capping.dir/build

examples/c/CMakeFiles/c-cloud-capping.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/examples/c && $(CMAKE_COMMAND) -P CMakeFiles/c-cloud-capping.dir/cmake_clean.cmake
.PHONY : examples/c/CMakeFiles/c-cloud-capping.dir/clean

examples/c/CMakeFiles/c-cloud-capping.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/examples/c /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/examples/c /github/simgrid/simgrid-3.26/build/examples/c/CMakeFiles/c-cloud-capping.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/c/CMakeFiles/c-cloud-capping.dir/depend

