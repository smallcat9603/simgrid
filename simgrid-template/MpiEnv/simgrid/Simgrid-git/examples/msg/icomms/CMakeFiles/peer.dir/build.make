# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.0

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
CMAKE_SOURCE_DIR = /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git

# Include any dependencies generated for this target.
include examples/msg/icomms/CMakeFiles/peer.dir/depend.make

# Include the progress variables for this target.
include examples/msg/icomms/CMakeFiles/peer.dir/progress.make

# Include the compile flags for this target's objects.
include examples/msg/icomms/CMakeFiles/peer.dir/flags.make

examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o: examples/msg/icomms/CMakeFiles/peer.dir/flags.make
examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o: examples/msg/icomms/peer.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/icomms && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/peer.dir/peer.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/icomms/peer.c

examples/msg/icomms/CMakeFiles/peer.dir/peer.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/peer.dir/peer.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/icomms && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/icomms/peer.c > CMakeFiles/peer.dir/peer.c.i

examples/msg/icomms/CMakeFiles/peer.dir/peer.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/peer.dir/peer.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/icomms && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/icomms/peer.c -o CMakeFiles/peer.dir/peer.c.s

examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o.requires:
.PHONY : examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o.requires

examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o.provides: examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o.requires
	$(MAKE) -f examples/msg/icomms/CMakeFiles/peer.dir/build.make examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o.provides.build
.PHONY : examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o.provides

examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o.provides.build: examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o

# Object files for target peer
peer_OBJECTS = \
"CMakeFiles/peer.dir/peer.c.o"

# External object files for target peer
peer_EXTERNAL_OBJECTS =

examples/msg/icomms/peer: examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o
examples/msg/icomms/peer: examples/msg/icomms/CMakeFiles/peer.dir/build.make
examples/msg/icomms/peer: lib/libsimgrid.so.3.12
examples/msg/icomms/peer: examples/msg/icomms/CMakeFiles/peer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable peer"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/icomms && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/peer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/msg/icomms/CMakeFiles/peer.dir/build: examples/msg/icomms/peer
.PHONY : examples/msg/icomms/CMakeFiles/peer.dir/build

examples/msg/icomms/CMakeFiles/peer.dir/requires: examples/msg/icomms/CMakeFiles/peer.dir/peer.c.o.requires
.PHONY : examples/msg/icomms/CMakeFiles/peer.dir/requires

examples/msg/icomms/CMakeFiles/peer.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/icomms && $(CMAKE_COMMAND) -P CMakeFiles/peer.dir/cmake_clean.cmake
.PHONY : examples/msg/icomms/CMakeFiles/peer.dir/clean

examples/msg/icomms/CMakeFiles/peer.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/icomms /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/icomms /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/icomms/CMakeFiles/peer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/msg/icomms/CMakeFiles/peer.dir/depend

