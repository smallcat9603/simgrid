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
include examples/c/CMakeFiles/c-cloud-masterworker.dir/depend.make

# Include the progress variables for this target.
include examples/c/CMakeFiles/c-cloud-masterworker.dir/progress.make

# Include the compile flags for this target's objects.
include examples/c/CMakeFiles/c-cloud-masterworker.dir/flags.make

examples/c/CMakeFiles/c-cloud-masterworker.dir/cloud-masterworker/cloud-masterworker.c.o: examples/c/CMakeFiles/c-cloud-masterworker.dir/flags.make
examples/c/CMakeFiles/c-cloud-masterworker.dir/cloud-masterworker/cloud-masterworker.c.o: ../examples/c/cloud-masterworker/cloud-masterworker.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object examples/c/CMakeFiles/c-cloud-masterworker.dir/cloud-masterworker/cloud-masterworker.c.o"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/c-cloud-masterworker.dir/cloud-masterworker/cloud-masterworker.c.o   -c /github/simgrid/simgrid-3.26/examples/c/cloud-masterworker/cloud-masterworker.c

examples/c/CMakeFiles/c-cloud-masterworker.dir/cloud-masterworker/cloud-masterworker.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/c-cloud-masterworker.dir/cloud-masterworker/cloud-masterworker.c.i"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/examples/c/cloud-masterworker/cloud-masterworker.c > CMakeFiles/c-cloud-masterworker.dir/cloud-masterworker/cloud-masterworker.c.i

examples/c/CMakeFiles/c-cloud-masterworker.dir/cloud-masterworker/cloud-masterworker.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/c-cloud-masterworker.dir/cloud-masterworker/cloud-masterworker.c.s"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/examples/c/cloud-masterworker/cloud-masterworker.c -o CMakeFiles/c-cloud-masterworker.dir/cloud-masterworker/cloud-masterworker.c.s

# Object files for target c-cloud-masterworker
c__cloud__masterworker_OBJECTS = \
"CMakeFiles/c-cloud-masterworker.dir/cloud-masterworker/cloud-masterworker.c.o"

# External object files for target c-cloud-masterworker
c__cloud__masterworker_EXTERNAL_OBJECTS =

examples/c/cloud-masterworker/c-cloud-masterworker: examples/c/CMakeFiles/c-cloud-masterworker.dir/cloud-masterworker/cloud-masterworker.c.o
examples/c/cloud-masterworker/c-cloud-masterworker: examples/c/CMakeFiles/c-cloud-masterworker.dir/build.make
examples/c/cloud-masterworker/c-cloud-masterworker: lib/libsimgrid.so.3.26
examples/c/cloud-masterworker/c-cloud-masterworker: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
examples/c/cloud-masterworker/c-cloud-masterworker: examples/c/CMakeFiles/c-cloud-masterworker.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable cloud-masterworker/c-cloud-masterworker"
	cd /github/simgrid/simgrid-3.26/build/examples/c && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/c-cloud-masterworker.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/c/CMakeFiles/c-cloud-masterworker.dir/build: examples/c/cloud-masterworker/c-cloud-masterworker

.PHONY : examples/c/CMakeFiles/c-cloud-masterworker.dir/build

examples/c/CMakeFiles/c-cloud-masterworker.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/examples/c && $(CMAKE_COMMAND) -P CMakeFiles/c-cloud-masterworker.dir/cmake_clean.cmake
.PHONY : examples/c/CMakeFiles/c-cloud-masterworker.dir/clean

examples/c/CMakeFiles/c-cloud-masterworker.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/examples/c /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/examples/c /github/simgrid/simgrid-3.26/build/examples/c/CMakeFiles/c-cloud-masterworker.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/c/CMakeFiles/c-cloud-masterworker.dir/depend

