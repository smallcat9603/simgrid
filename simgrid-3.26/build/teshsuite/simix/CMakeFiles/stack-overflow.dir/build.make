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
include teshsuite/simix/CMakeFiles/stack-overflow.dir/depend.make

# Include the progress variables for this target.
include teshsuite/simix/CMakeFiles/stack-overflow.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/simix/CMakeFiles/stack-overflow.dir/flags.make

teshsuite/simix/CMakeFiles/stack-overflow.dir/stack-overflow/stack-overflow.cpp.o: teshsuite/simix/CMakeFiles/stack-overflow.dir/flags.make
teshsuite/simix/CMakeFiles/stack-overflow.dir/stack-overflow/stack-overflow.cpp.o: ../teshsuite/simix/stack-overflow/stack-overflow.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object teshsuite/simix/CMakeFiles/stack-overflow.dir/stack-overflow/stack-overflow.cpp.o"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simix && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/stack-overflow.dir/stack-overflow/stack-overflow.cpp.o -c /github/simgrid/simgrid-3.26/teshsuite/simix/stack-overflow/stack-overflow.cpp

teshsuite/simix/CMakeFiles/stack-overflow.dir/stack-overflow/stack-overflow.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/stack-overflow.dir/stack-overflow/stack-overflow.cpp.i"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simix && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /github/simgrid/simgrid-3.26/teshsuite/simix/stack-overflow/stack-overflow.cpp > CMakeFiles/stack-overflow.dir/stack-overflow/stack-overflow.cpp.i

teshsuite/simix/CMakeFiles/stack-overflow.dir/stack-overflow/stack-overflow.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/stack-overflow.dir/stack-overflow/stack-overflow.cpp.s"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simix && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /github/simgrid/simgrid-3.26/teshsuite/simix/stack-overflow/stack-overflow.cpp -o CMakeFiles/stack-overflow.dir/stack-overflow/stack-overflow.cpp.s

# Object files for target stack-overflow
stack__overflow_OBJECTS = \
"CMakeFiles/stack-overflow.dir/stack-overflow/stack-overflow.cpp.o"

# External object files for target stack-overflow
stack__overflow_EXTERNAL_OBJECTS =

teshsuite/simix/stack-overflow/stack-overflow: teshsuite/simix/CMakeFiles/stack-overflow.dir/stack-overflow/stack-overflow.cpp.o
teshsuite/simix/stack-overflow/stack-overflow: teshsuite/simix/CMakeFiles/stack-overflow.dir/build.make
teshsuite/simix/stack-overflow/stack-overflow: lib/libsimgrid.so.3.26
teshsuite/simix/stack-overflow/stack-overflow: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
teshsuite/simix/stack-overflow/stack-overflow: teshsuite/simix/CMakeFiles/stack-overflow.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable stack-overflow/stack-overflow"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simix && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/stack-overflow.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/simix/CMakeFiles/stack-overflow.dir/build: teshsuite/simix/stack-overflow/stack-overflow

.PHONY : teshsuite/simix/CMakeFiles/stack-overflow.dir/build

teshsuite/simix/CMakeFiles/stack-overflow.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/teshsuite/simix && $(CMAKE_COMMAND) -P CMakeFiles/stack-overflow.dir/cmake_clean.cmake
.PHONY : teshsuite/simix/CMakeFiles/stack-overflow.dir/clean

teshsuite/simix/CMakeFiles/stack-overflow.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/teshsuite/simix /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/teshsuite/simix /github/simgrid/simgrid-3.26/build/teshsuite/simix/CMakeFiles/stack-overflow.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/simix/CMakeFiles/stack-overflow.dir/depend
