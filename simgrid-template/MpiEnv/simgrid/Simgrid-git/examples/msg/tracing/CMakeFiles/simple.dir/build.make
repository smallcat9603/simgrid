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
include examples/msg/tracing/CMakeFiles/simple.dir/depend.make

# Include the progress variables for this target.
include examples/msg/tracing/CMakeFiles/simple.dir/progress.make

# Include the compile flags for this target's objects.
include examples/msg/tracing/CMakeFiles/simple.dir/flags.make

examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o: examples/msg/tracing/CMakeFiles/simple.dir/flags.make
examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o: examples/msg/tracing/simple.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/tracing && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/simple.dir/simple.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/tracing/simple.c

examples/msg/tracing/CMakeFiles/simple.dir/simple.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/simple.dir/simple.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/tracing && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/tracing/simple.c > CMakeFiles/simple.dir/simple.c.i

examples/msg/tracing/CMakeFiles/simple.dir/simple.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/simple.dir/simple.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/tracing && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/tracing/simple.c -o CMakeFiles/simple.dir/simple.c.s

examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o.requires:
.PHONY : examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o.requires

examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o.provides: examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o.requires
	$(MAKE) -f examples/msg/tracing/CMakeFiles/simple.dir/build.make examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o.provides.build
.PHONY : examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o.provides

examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o.provides.build: examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o

# Object files for target simple
simple_OBJECTS = \
"CMakeFiles/simple.dir/simple.c.o"

# External object files for target simple
simple_EXTERNAL_OBJECTS =

examples/msg/tracing/simple: examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o
examples/msg/tracing/simple: examples/msg/tracing/CMakeFiles/simple.dir/build.make
examples/msg/tracing/simple: lib/libsimgrid.so.3.12
examples/msg/tracing/simple: examples/msg/tracing/CMakeFiles/simple.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable simple"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/tracing && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simple.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/msg/tracing/CMakeFiles/simple.dir/build: examples/msg/tracing/simple
.PHONY : examples/msg/tracing/CMakeFiles/simple.dir/build

examples/msg/tracing/CMakeFiles/simple.dir/requires: examples/msg/tracing/CMakeFiles/simple.dir/simple.c.o.requires
.PHONY : examples/msg/tracing/CMakeFiles/simple.dir/requires

examples/msg/tracing/CMakeFiles/simple.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/tracing && $(CMAKE_COMMAND) -P CMakeFiles/simple.dir/cmake_clean.cmake
.PHONY : examples/msg/tracing/CMakeFiles/simple.dir/clean

examples/msg/tracing/CMakeFiles/simple.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/tracing /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/tracing /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/tracing/CMakeFiles/simple.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/msg/tracing/CMakeFiles/simple.dir/depend

