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
include teshsuite/xbt/CMakeFiles/log_large.dir/depend.make

# Include the progress variables for this target.
include teshsuite/xbt/CMakeFiles/log_large.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/xbt/CMakeFiles/log_large.dir/flags.make

teshsuite/xbt/CMakeFiles/log_large.dir/log_large/log_large.c.o: teshsuite/xbt/CMakeFiles/log_large.dir/flags.make
teshsuite/xbt/CMakeFiles/log_large.dir/log_large/log_large.c.o: ../teshsuite/xbt/log_large/log_large.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object teshsuite/xbt/CMakeFiles/log_large.dir/log_large/log_large.c.o"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/xbt && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/log_large.dir/log_large/log_large.c.o   -c /github/simgrid/simgrid-3.26/teshsuite/xbt/log_large/log_large.c

teshsuite/xbt/CMakeFiles/log_large.dir/log_large/log_large.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/log_large.dir/log_large/log_large.c.i"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/xbt && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/teshsuite/xbt/log_large/log_large.c > CMakeFiles/log_large.dir/log_large/log_large.c.i

teshsuite/xbt/CMakeFiles/log_large.dir/log_large/log_large.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/log_large.dir/log_large/log_large.c.s"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/xbt && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/teshsuite/xbt/log_large/log_large.c -o CMakeFiles/log_large.dir/log_large/log_large.c.s

# Object files for target log_large
log_large_OBJECTS = \
"CMakeFiles/log_large.dir/log_large/log_large.c.o"

# External object files for target log_large
log_large_EXTERNAL_OBJECTS =

teshsuite/xbt/log_large/log_large: teshsuite/xbt/CMakeFiles/log_large.dir/log_large/log_large.c.o
teshsuite/xbt/log_large/log_large: teshsuite/xbt/CMakeFiles/log_large.dir/build.make
teshsuite/xbt/log_large/log_large: lib/libsimgrid.so.3.26
teshsuite/xbt/log_large/log_large: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
teshsuite/xbt/log_large/log_large: teshsuite/xbt/CMakeFiles/log_large.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable log_large/log_large"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/xbt && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/log_large.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/xbt/CMakeFiles/log_large.dir/build: teshsuite/xbt/log_large/log_large

.PHONY : teshsuite/xbt/CMakeFiles/log_large.dir/build

teshsuite/xbt/CMakeFiles/log_large.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/teshsuite/xbt && $(CMAKE_COMMAND) -P CMakeFiles/log_large.dir/cmake_clean.cmake
.PHONY : teshsuite/xbt/CMakeFiles/log_large.dir/clean

teshsuite/xbt/CMakeFiles/log_large.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/teshsuite/xbt /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/teshsuite/xbt /github/simgrid/simgrid-3.26/build/teshsuite/xbt/CMakeFiles/log_large.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/xbt/CMakeFiles/log_large.dir/depend

