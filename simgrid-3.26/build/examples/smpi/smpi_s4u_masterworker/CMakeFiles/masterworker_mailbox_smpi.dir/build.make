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
include examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/depend.make

# Include the progress variables for this target.
include examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/progress.make

# Include the compile flags for this target's objects.
include examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/flags.make

examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/masterworker_mailbox_smpi.cpp.o: examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/flags.make
examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/masterworker_mailbox_smpi.cpp.o: ../examples/smpi/smpi_s4u_masterworker/masterworker_mailbox_smpi.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/masterworker_mailbox_smpi.cpp.o"
	cd /github/simgrid/simgrid-3.26/build/examples/smpi/smpi_s4u_masterworker && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/masterworker_mailbox_smpi.dir/masterworker_mailbox_smpi.cpp.o -c /github/simgrid/simgrid-3.26/examples/smpi/smpi_s4u_masterworker/masterworker_mailbox_smpi.cpp

examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/masterworker_mailbox_smpi.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/masterworker_mailbox_smpi.dir/masterworker_mailbox_smpi.cpp.i"
	cd /github/simgrid/simgrid-3.26/build/examples/smpi/smpi_s4u_masterworker && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /github/simgrid/simgrid-3.26/examples/smpi/smpi_s4u_masterworker/masterworker_mailbox_smpi.cpp > CMakeFiles/masterworker_mailbox_smpi.dir/masterworker_mailbox_smpi.cpp.i

examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/masterworker_mailbox_smpi.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/masterworker_mailbox_smpi.dir/masterworker_mailbox_smpi.cpp.s"
	cd /github/simgrid/simgrid-3.26/build/examples/smpi/smpi_s4u_masterworker && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /github/simgrid/simgrid-3.26/examples/smpi/smpi_s4u_masterworker/masterworker_mailbox_smpi.cpp -o CMakeFiles/masterworker_mailbox_smpi.dir/masterworker_mailbox_smpi.cpp.s

# Object files for target masterworker_mailbox_smpi
masterworker_mailbox_smpi_OBJECTS = \
"CMakeFiles/masterworker_mailbox_smpi.dir/masterworker_mailbox_smpi.cpp.o"

# External object files for target masterworker_mailbox_smpi
masterworker_mailbox_smpi_EXTERNAL_OBJECTS =

examples/smpi/smpi_s4u_masterworker/masterworker_mailbox_smpi: examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/masterworker_mailbox_smpi.cpp.o
examples/smpi/smpi_s4u_masterworker/masterworker_mailbox_smpi: examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/build.make
examples/smpi/smpi_s4u_masterworker/masterworker_mailbox_smpi: lib/libsimgrid.so.3.26
examples/smpi/smpi_s4u_masterworker/masterworker_mailbox_smpi: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
examples/smpi/smpi_s4u_masterworker/masterworker_mailbox_smpi: examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable masterworker_mailbox_smpi"
	cd /github/simgrid/simgrid-3.26/build/examples/smpi/smpi_s4u_masterworker && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/masterworker_mailbox_smpi.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/build: examples/smpi/smpi_s4u_masterworker/masterworker_mailbox_smpi

.PHONY : examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/build

examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/examples/smpi/smpi_s4u_masterworker && $(CMAKE_COMMAND) -P CMakeFiles/masterworker_mailbox_smpi.dir/cmake_clean.cmake
.PHONY : examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/clean

examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/examples/smpi/smpi_s4u_masterworker /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/examples/smpi/smpi_s4u_masterworker /github/simgrid/simgrid-3.26/build/examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/smpi/smpi_s4u_masterworker/CMakeFiles/masterworker_mailbox_smpi.dir/depend

