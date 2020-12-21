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
include examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/depend.make

# Include the progress variables for this target.
include examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/progress.make

# Include the compile flags for this target's objects.
include examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/flags.make

examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/exec-dvfs/s4u-exec-dvfs.cpp.o: examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/flags.make
examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/exec-dvfs/s4u-exec-dvfs.cpp.o: ../examples/s4u/exec-dvfs/s4u-exec-dvfs.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/exec-dvfs/s4u-exec-dvfs.cpp.o"
	cd /github/simgrid/simgrid-3.26/build/examples/s4u && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/s4u-exec-dvfs.dir/exec-dvfs/s4u-exec-dvfs.cpp.o -c /github/simgrid/simgrid-3.26/examples/s4u/exec-dvfs/s4u-exec-dvfs.cpp

examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/exec-dvfs/s4u-exec-dvfs.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/s4u-exec-dvfs.dir/exec-dvfs/s4u-exec-dvfs.cpp.i"
	cd /github/simgrid/simgrid-3.26/build/examples/s4u && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /github/simgrid/simgrid-3.26/examples/s4u/exec-dvfs/s4u-exec-dvfs.cpp > CMakeFiles/s4u-exec-dvfs.dir/exec-dvfs/s4u-exec-dvfs.cpp.i

examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/exec-dvfs/s4u-exec-dvfs.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/s4u-exec-dvfs.dir/exec-dvfs/s4u-exec-dvfs.cpp.s"
	cd /github/simgrid/simgrid-3.26/build/examples/s4u && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /github/simgrid/simgrid-3.26/examples/s4u/exec-dvfs/s4u-exec-dvfs.cpp -o CMakeFiles/s4u-exec-dvfs.dir/exec-dvfs/s4u-exec-dvfs.cpp.s

# Object files for target s4u-exec-dvfs
s4u__exec__dvfs_OBJECTS = \
"CMakeFiles/s4u-exec-dvfs.dir/exec-dvfs/s4u-exec-dvfs.cpp.o"

# External object files for target s4u-exec-dvfs
s4u__exec__dvfs_EXTERNAL_OBJECTS =

examples/s4u/exec-dvfs/s4u-exec-dvfs: examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/exec-dvfs/s4u-exec-dvfs.cpp.o
examples/s4u/exec-dvfs/s4u-exec-dvfs: examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/build.make
examples/s4u/exec-dvfs/s4u-exec-dvfs: lib/libsimgrid.so.3.26
examples/s4u/exec-dvfs/s4u-exec-dvfs: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
examples/s4u/exec-dvfs/s4u-exec-dvfs: examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable exec-dvfs/s4u-exec-dvfs"
	cd /github/simgrid/simgrid-3.26/build/examples/s4u && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/s4u-exec-dvfs.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/build: examples/s4u/exec-dvfs/s4u-exec-dvfs

.PHONY : examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/build

examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/examples/s4u && $(CMAKE_COMMAND) -P CMakeFiles/s4u-exec-dvfs.dir/cmake_clean.cmake
.PHONY : examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/clean

examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/examples/s4u /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/examples/s4u /github/simgrid/simgrid-3.26/build/examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/s4u/CMakeFiles/s4u-exec-dvfs.dir/depend

