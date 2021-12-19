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
include teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/depend.make

# Include the progress variables for this target.
include teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/flags.make

teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o: teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/flags.make
teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o: teshsuite/smpi/barrier/barrier.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/barrier && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/barrier_coll.dir/barrier.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/barrier/barrier.c

teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/barrier_coll.dir/barrier.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/barrier && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/barrier/barrier.c > CMakeFiles/barrier_coll.dir/barrier.c.i

teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/barrier_coll.dir/barrier.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/barrier && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/barrier/barrier.c -o CMakeFiles/barrier_coll.dir/barrier.c.s

teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o.requires:
.PHONY : teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o.requires

teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o.provides: teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o.requires
	$(MAKE) -f teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/build.make teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o.provides.build
.PHONY : teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o.provides

teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o.provides.build: teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o

# Object files for target barrier_coll
barrier_coll_OBJECTS = \
"CMakeFiles/barrier_coll.dir/barrier.c.o"

# External object files for target barrier_coll
barrier_coll_EXTERNAL_OBJECTS =

teshsuite/smpi/barrier/barrier_coll: teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o
teshsuite/smpi/barrier/barrier_coll: teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/build.make
teshsuite/smpi/barrier/barrier_coll: lib/libsimgrid.so.3.12
teshsuite/smpi/barrier/barrier_coll: teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable barrier_coll"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/barrier && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/barrier_coll.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/build: teshsuite/smpi/barrier/barrier_coll
.PHONY : teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/build

teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/requires: teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/barrier.c.o.requires
.PHONY : teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/requires

teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/barrier && $(CMAKE_COMMAND) -P CMakeFiles/barrier_coll.dir/cmake_clean.cmake
.PHONY : teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/clean

teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/barrier /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/barrier /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/smpi/barrier/CMakeFiles/barrier_coll.dir/depend
