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
include teshsuite/simdag/CMakeFiles/comp-only-seq.dir/depend.make

# Include the progress variables for this target.
include teshsuite/simdag/CMakeFiles/comp-only-seq.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/simdag/CMakeFiles/comp-only-seq.dir/flags.make

teshsuite/simdag/CMakeFiles/comp-only-seq.dir/comp-only-seq/comp-only-seq.c.o: teshsuite/simdag/CMakeFiles/comp-only-seq.dir/flags.make
teshsuite/simdag/CMakeFiles/comp-only-seq.dir/comp-only-seq/comp-only-seq.c.o: ../teshsuite/simdag/comp-only-seq/comp-only-seq.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object teshsuite/simdag/CMakeFiles/comp-only-seq.dir/comp-only-seq/comp-only-seq.c.o"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simdag && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/comp-only-seq.dir/comp-only-seq/comp-only-seq.c.o   -c /github/simgrid/simgrid-3.26/teshsuite/simdag/comp-only-seq/comp-only-seq.c

teshsuite/simdag/CMakeFiles/comp-only-seq.dir/comp-only-seq/comp-only-seq.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/comp-only-seq.dir/comp-only-seq/comp-only-seq.c.i"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simdag && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/teshsuite/simdag/comp-only-seq/comp-only-seq.c > CMakeFiles/comp-only-seq.dir/comp-only-seq/comp-only-seq.c.i

teshsuite/simdag/CMakeFiles/comp-only-seq.dir/comp-only-seq/comp-only-seq.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/comp-only-seq.dir/comp-only-seq/comp-only-seq.c.s"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simdag && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/teshsuite/simdag/comp-only-seq/comp-only-seq.c -o CMakeFiles/comp-only-seq.dir/comp-only-seq/comp-only-seq.c.s

# Object files for target comp-only-seq
comp__only__seq_OBJECTS = \
"CMakeFiles/comp-only-seq.dir/comp-only-seq/comp-only-seq.c.o"

# External object files for target comp-only-seq
comp__only__seq_EXTERNAL_OBJECTS =

teshsuite/simdag/comp-only-seq/comp-only-seq: teshsuite/simdag/CMakeFiles/comp-only-seq.dir/comp-only-seq/comp-only-seq.c.o
teshsuite/simdag/comp-only-seq/comp-only-seq: teshsuite/simdag/CMakeFiles/comp-only-seq.dir/build.make
teshsuite/simdag/comp-only-seq/comp-only-seq: lib/libsimgrid.so.3.26
teshsuite/simdag/comp-only-seq/comp-only-seq: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
teshsuite/simdag/comp-only-seq/comp-only-seq: teshsuite/simdag/CMakeFiles/comp-only-seq.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable comp-only-seq/comp-only-seq"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simdag && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/comp-only-seq.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/simdag/CMakeFiles/comp-only-seq.dir/build: teshsuite/simdag/comp-only-seq/comp-only-seq

.PHONY : teshsuite/simdag/CMakeFiles/comp-only-seq.dir/build

teshsuite/simdag/CMakeFiles/comp-only-seq.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simdag && $(CMAKE_COMMAND) -P CMakeFiles/comp-only-seq.dir/cmake_clean.cmake
.PHONY : teshsuite/simdag/CMakeFiles/comp-only-seq.dir/clean

teshsuite/simdag/CMakeFiles/comp-only-seq.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/teshsuite/simdag /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/teshsuite/simdag /github/simgrid/simgrid-3.26/build/teshsuite/simdag/CMakeFiles/comp-only-seq.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/simdag/CMakeFiles/comp-only-seq.dir/depend

