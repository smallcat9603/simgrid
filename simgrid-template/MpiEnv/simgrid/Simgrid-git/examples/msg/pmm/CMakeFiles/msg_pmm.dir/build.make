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
include examples/msg/pmm/CMakeFiles/msg_pmm.dir/depend.make

# Include the progress variables for this target.
include examples/msg/pmm/CMakeFiles/msg_pmm.dir/progress.make

# Include the compile flags for this target's objects.
include examples/msg/pmm/CMakeFiles/msg_pmm.dir/flags.make

examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o: examples/msg/pmm/CMakeFiles/msg_pmm.dir/flags.make
examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o: examples/msg/pmm/msg_pmm.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/pmm && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/msg_pmm.dir/msg_pmm.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/pmm/msg_pmm.c

examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/msg_pmm.dir/msg_pmm.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/pmm && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/pmm/msg_pmm.c > CMakeFiles/msg_pmm.dir/msg_pmm.c.i

examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/msg_pmm.dir/msg_pmm.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/pmm && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/pmm/msg_pmm.c -o CMakeFiles/msg_pmm.dir/msg_pmm.c.s

examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o.requires:
.PHONY : examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o.requires

examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o.provides: examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o.requires
	$(MAKE) -f examples/msg/pmm/CMakeFiles/msg_pmm.dir/build.make examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o.provides.build
.PHONY : examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o.provides

examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o.provides.build: examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o

# Object files for target msg_pmm
msg_pmm_OBJECTS = \
"CMakeFiles/msg_pmm.dir/msg_pmm.c.o"

# External object files for target msg_pmm
msg_pmm_EXTERNAL_OBJECTS =

examples/msg/pmm/msg_pmm: examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o
examples/msg/pmm/msg_pmm: examples/msg/pmm/CMakeFiles/msg_pmm.dir/build.make
examples/msg/pmm/msg_pmm: lib/libsimgrid.so.3.12
examples/msg/pmm/msg_pmm: examples/msg/pmm/CMakeFiles/msg_pmm.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable msg_pmm"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/pmm && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/msg_pmm.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/msg/pmm/CMakeFiles/msg_pmm.dir/build: examples/msg/pmm/msg_pmm
.PHONY : examples/msg/pmm/CMakeFiles/msg_pmm.dir/build

examples/msg/pmm/CMakeFiles/msg_pmm.dir/requires: examples/msg/pmm/CMakeFiles/msg_pmm.dir/msg_pmm.c.o.requires
.PHONY : examples/msg/pmm/CMakeFiles/msg_pmm.dir/requires

examples/msg/pmm/CMakeFiles/msg_pmm.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/pmm && $(CMAKE_COMMAND) -P CMakeFiles/msg_pmm.dir/cmake_clean.cmake
.PHONY : examples/msg/pmm/CMakeFiles/msg_pmm.dir/clean

examples/msg/pmm/CMakeFiles/msg_pmm.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/pmm /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/pmm /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/pmm/CMakeFiles/msg_pmm.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/msg/pmm/CMakeFiles/msg_pmm.dir/depend
