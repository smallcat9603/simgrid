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
include examples/smpi/energy/CMakeFiles/se.dir/depend.make

# Include the progress variables for this target.
include examples/smpi/energy/CMakeFiles/se.dir/progress.make

# Include the compile flags for this target's objects.
include examples/smpi/energy/CMakeFiles/se.dir/flags.make

examples/smpi/energy/CMakeFiles/se.dir/se.c.o: examples/smpi/energy/CMakeFiles/se.dir/flags.make
examples/smpi/energy/CMakeFiles/se.dir/se.c.o: examples/smpi/energy/se.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object examples/smpi/energy/CMakeFiles/se.dir/se.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/smpi/energy && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/se.dir/se.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/smpi/energy/se.c

examples/smpi/energy/CMakeFiles/se.dir/se.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/se.dir/se.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/smpi/energy && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/smpi/energy/se.c > CMakeFiles/se.dir/se.c.i

examples/smpi/energy/CMakeFiles/se.dir/se.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/se.dir/se.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/smpi/energy && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/smpi/energy/se.c -o CMakeFiles/se.dir/se.c.s

examples/smpi/energy/CMakeFiles/se.dir/se.c.o.requires:
.PHONY : examples/smpi/energy/CMakeFiles/se.dir/se.c.o.requires

examples/smpi/energy/CMakeFiles/se.dir/se.c.o.provides: examples/smpi/energy/CMakeFiles/se.dir/se.c.o.requires
	$(MAKE) -f examples/smpi/energy/CMakeFiles/se.dir/build.make examples/smpi/energy/CMakeFiles/se.dir/se.c.o.provides.build
.PHONY : examples/smpi/energy/CMakeFiles/se.dir/se.c.o.provides

examples/smpi/energy/CMakeFiles/se.dir/se.c.o.provides.build: examples/smpi/energy/CMakeFiles/se.dir/se.c.o

# Object files for target se
se_OBJECTS = \
"CMakeFiles/se.dir/se.c.o"

# External object files for target se
se_EXTERNAL_OBJECTS =

examples/smpi/energy/se: examples/smpi/energy/CMakeFiles/se.dir/se.c.o
examples/smpi/energy/se: examples/smpi/energy/CMakeFiles/se.dir/build.make
examples/smpi/energy/se: lib/libsimgrid.so.3.12
examples/smpi/energy/se: examples/smpi/energy/CMakeFiles/se.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable se"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/smpi/energy && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/se.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/smpi/energy/CMakeFiles/se.dir/build: examples/smpi/energy/se
.PHONY : examples/smpi/energy/CMakeFiles/se.dir/build

examples/smpi/energy/CMakeFiles/se.dir/requires: examples/smpi/energy/CMakeFiles/se.dir/se.c.o.requires
.PHONY : examples/smpi/energy/CMakeFiles/se.dir/requires

examples/smpi/energy/CMakeFiles/se.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/smpi/energy && $(CMAKE_COMMAND) -P CMakeFiles/se.dir/cmake_clean.cmake
.PHONY : examples/smpi/energy/CMakeFiles/se.dir/clean

examples/smpi/energy/CMakeFiles/se.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/smpi/energy /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/smpi/energy /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/smpi/energy/CMakeFiles/se.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/smpi/energy/CMakeFiles/se.dir/depend
