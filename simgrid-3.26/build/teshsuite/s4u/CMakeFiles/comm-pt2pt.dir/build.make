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
include teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/depend.make

# Include the progress variables for this target.
include teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/flags.make

teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/comm-pt2pt/comm-pt2pt.cpp.o: teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/flags.make
teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/comm-pt2pt/comm-pt2pt.cpp.o: ../teshsuite/s4u/comm-pt2pt/comm-pt2pt.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/comm-pt2pt/comm-pt2pt.cpp.o"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/s4u && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/comm-pt2pt.dir/comm-pt2pt/comm-pt2pt.cpp.o -c /github/simgrid/simgrid-3.26/teshsuite/s4u/comm-pt2pt/comm-pt2pt.cpp

teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/comm-pt2pt/comm-pt2pt.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/comm-pt2pt.dir/comm-pt2pt/comm-pt2pt.cpp.i"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/s4u && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /github/simgrid/simgrid-3.26/teshsuite/s4u/comm-pt2pt/comm-pt2pt.cpp > CMakeFiles/comm-pt2pt.dir/comm-pt2pt/comm-pt2pt.cpp.i

teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/comm-pt2pt/comm-pt2pt.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/comm-pt2pt.dir/comm-pt2pt/comm-pt2pt.cpp.s"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/s4u && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /github/simgrid/simgrid-3.26/teshsuite/s4u/comm-pt2pt/comm-pt2pt.cpp -o CMakeFiles/comm-pt2pt.dir/comm-pt2pt/comm-pt2pt.cpp.s

# Object files for target comm-pt2pt
comm__pt2pt_OBJECTS = \
"CMakeFiles/comm-pt2pt.dir/comm-pt2pt/comm-pt2pt.cpp.o"

# External object files for target comm-pt2pt
comm__pt2pt_EXTERNAL_OBJECTS =

teshsuite/s4u/comm-pt2pt/comm-pt2pt: teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/comm-pt2pt/comm-pt2pt.cpp.o
teshsuite/s4u/comm-pt2pt/comm-pt2pt: teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/build.make
teshsuite/s4u/comm-pt2pt/comm-pt2pt: lib/libsimgrid.so.3.26
teshsuite/s4u/comm-pt2pt/comm-pt2pt: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
teshsuite/s4u/comm-pt2pt/comm-pt2pt: teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable comm-pt2pt/comm-pt2pt"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/s4u && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/comm-pt2pt.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/build: teshsuite/s4u/comm-pt2pt/comm-pt2pt

.PHONY : teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/build

teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/teshsuite/s4u && $(CMAKE_COMMAND) -P CMakeFiles/comm-pt2pt.dir/cmake_clean.cmake
.PHONY : teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/clean

teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/teshsuite/s4u /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/teshsuite/s4u /github/simgrid/simgrid-3.26/build/teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/s4u/CMakeFiles/comm-pt2pt.dir/depend
