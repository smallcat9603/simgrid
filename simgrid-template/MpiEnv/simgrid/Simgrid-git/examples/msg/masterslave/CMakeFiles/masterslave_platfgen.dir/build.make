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
include examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/depend.make

# Include the progress variables for this target.
include examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/progress.make

# Include the compile flags for this target's objects.
include examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/flags.make

examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o: examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/flags.make
examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o: examples/msg/masterslave/masterslave_platfgen.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/masterslave && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/masterslave/masterslave_platfgen.c

examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/masterslave && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/masterslave/masterslave_platfgen.c > CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.i

examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/masterslave && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/masterslave/masterslave_platfgen.c -o CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.s

examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o.requires:
.PHONY : examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o.requires

examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o.provides: examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o.requires
	$(MAKE) -f examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/build.make examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o.provides.build
.PHONY : examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o.provides

examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o.provides.build: examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o

# Object files for target masterslave_platfgen
masterslave_platfgen_OBJECTS = \
"CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o"

# External object files for target masterslave_platfgen
masterslave_platfgen_EXTERNAL_OBJECTS =

examples/msg/masterslave/masterslave_platfgen: examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o
examples/msg/masterslave/masterslave_platfgen: examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/build.make
examples/msg/masterslave/masterslave_platfgen: lib/libsimgrid.so.3.12
examples/msg/masterslave/masterslave_platfgen: examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable masterslave_platfgen"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/masterslave && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/masterslave_platfgen.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/build: examples/msg/masterslave/masterslave_platfgen
.PHONY : examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/build

examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/requires: examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/masterslave_platfgen.c.o.requires
.PHONY : examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/requires

examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/masterslave && $(CMAKE_COMMAND) -P CMakeFiles/masterslave_platfgen.dir/cmake_clean.cmake
.PHONY : examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/clean

examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/masterslave /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/masterslave /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/msg/masterslave/CMakeFiles/masterslave_platfgen.dir/depend

