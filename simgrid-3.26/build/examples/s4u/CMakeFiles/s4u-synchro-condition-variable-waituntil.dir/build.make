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
include examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/depend.make

# Include the progress variables for this target.
include examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/progress.make

# Include the compile flags for this target's objects.
include examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/flags.make

examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp.o: examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/flags.make
examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp.o: ../examples/s4u/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp.o"
	cd /github/simgrid/simgrid-3.26/build/examples/s4u && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp.o -c /github/simgrid/simgrid-3.26/examples/s4u/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp

examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp.i"
	cd /github/simgrid/simgrid-3.26/build/examples/s4u && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /github/simgrid/simgrid-3.26/examples/s4u/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp > CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp.i

examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp.s"
	cd /github/simgrid/simgrid-3.26/build/examples/s4u && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /github/simgrid/simgrid-3.26/examples/s4u/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp -o CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp.s

# Object files for target s4u-synchro-condition-variable-waituntil
s4u__synchro__condition__variable__waituntil_OBJECTS = \
"CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp.o"

# External object files for target s4u-synchro-condition-variable-waituntil
s4u__synchro__condition__variable__waituntil_EXTERNAL_OBJECTS =

examples/s4u/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil: examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil.cpp.o
examples/s4u/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil: examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/build.make
examples/s4u/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil: lib/libsimgrid.so.3.26
examples/s4u/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
examples/s4u/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil: examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil"
	cd /github/simgrid/simgrid-3.26/build/examples/s4u && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/build: examples/s4u/synchro-condition-variable-waituntil/s4u-synchro-condition-variable-waituntil

.PHONY : examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/build

examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/examples/s4u && $(CMAKE_COMMAND) -P CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/cmake_clean.cmake
.PHONY : examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/clean

examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/examples/s4u /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/examples/s4u /github/simgrid/simgrid-3.26/build/examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/s4u/CMakeFiles/s4u-synchro-condition-variable-waituntil.dir/depend

