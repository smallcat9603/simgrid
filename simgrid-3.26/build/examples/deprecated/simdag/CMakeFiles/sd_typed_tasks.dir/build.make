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
include examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/depend.make

# Include the progress variables for this target.
include examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/progress.make

# Include the compile flags for this target's objects.
include examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/flags.make

examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/typed_tasks/sd_typed_tasks.c.o: examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/flags.make
examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/typed_tasks/sd_typed_tasks.c.o: ../examples/deprecated/simdag/typed_tasks/sd_typed_tasks.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/typed_tasks/sd_typed_tasks.c.o"
	cd /github/simgrid/simgrid-3.26/build/examples/deprecated/simdag && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/sd_typed_tasks.dir/typed_tasks/sd_typed_tasks.c.o   -c /github/simgrid/simgrid-3.26/examples/deprecated/simdag/typed_tasks/sd_typed_tasks.c

examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/typed_tasks/sd_typed_tasks.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/sd_typed_tasks.dir/typed_tasks/sd_typed_tasks.c.i"
	cd /github/simgrid/simgrid-3.26/build/examples/deprecated/simdag && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/examples/deprecated/simdag/typed_tasks/sd_typed_tasks.c > CMakeFiles/sd_typed_tasks.dir/typed_tasks/sd_typed_tasks.c.i

examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/typed_tasks/sd_typed_tasks.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/sd_typed_tasks.dir/typed_tasks/sd_typed_tasks.c.s"
	cd /github/simgrid/simgrid-3.26/build/examples/deprecated/simdag && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/examples/deprecated/simdag/typed_tasks/sd_typed_tasks.c -o CMakeFiles/sd_typed_tasks.dir/typed_tasks/sd_typed_tasks.c.s

# Object files for target sd_typed_tasks
sd_typed_tasks_OBJECTS = \
"CMakeFiles/sd_typed_tasks.dir/typed_tasks/sd_typed_tasks.c.o"

# External object files for target sd_typed_tasks
sd_typed_tasks_EXTERNAL_OBJECTS =

examples/deprecated/simdag/typed_tasks/sd_typed_tasks: examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/typed_tasks/sd_typed_tasks.c.o
examples/deprecated/simdag/typed_tasks/sd_typed_tasks: examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/build.make
examples/deprecated/simdag/typed_tasks/sd_typed_tasks: lib/libsimgrid.so.3.26
examples/deprecated/simdag/typed_tasks/sd_typed_tasks: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
examples/deprecated/simdag/typed_tasks/sd_typed_tasks: examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable typed_tasks/sd_typed_tasks"
	cd /github/simgrid/simgrid-3.26/build/examples/deprecated/simdag && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sd_typed_tasks.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/build: examples/deprecated/simdag/typed_tasks/sd_typed_tasks

.PHONY : examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/build

examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/examples/deprecated/simdag && $(CMAKE_COMMAND) -P CMakeFiles/sd_typed_tasks.dir/cmake_clean.cmake
.PHONY : examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/clean

examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/examples/deprecated/simdag /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/examples/deprecated/simdag /github/simgrid/simgrid-3.26/build/examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/deprecated/simdag/CMakeFiles/sd_typed_tasks.dir/depend

