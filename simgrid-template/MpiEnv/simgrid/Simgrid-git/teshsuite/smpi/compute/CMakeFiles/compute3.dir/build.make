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
include teshsuite/smpi/compute/CMakeFiles/compute3.dir/depend.make

# Include the progress variables for this target.
include teshsuite/smpi/compute/CMakeFiles/compute3.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/smpi/compute/CMakeFiles/compute3.dir/flags.make

teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o: teshsuite/smpi/compute/CMakeFiles/compute3.dir/flags.make
teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o: teshsuite/smpi/compute/compute3.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/compute && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/compute3.dir/compute3.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/compute/compute3.c

teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/compute3.dir/compute3.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/compute && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/compute/compute3.c > CMakeFiles/compute3.dir/compute3.c.i

teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/compute3.dir/compute3.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/compute && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/compute/compute3.c -o CMakeFiles/compute3.dir/compute3.c.s

teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o.requires:
.PHONY : teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o.requires

teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o.provides: teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o.requires
	$(MAKE) -f teshsuite/smpi/compute/CMakeFiles/compute3.dir/build.make teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o.provides.build
.PHONY : teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o.provides

teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o.provides.build: teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o

# Object files for target compute3
compute3_OBJECTS = \
"CMakeFiles/compute3.dir/compute3.c.o"

# External object files for target compute3
compute3_EXTERNAL_OBJECTS =

teshsuite/smpi/compute/compute3: teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o
teshsuite/smpi/compute/compute3: teshsuite/smpi/compute/CMakeFiles/compute3.dir/build.make
teshsuite/smpi/compute/compute3: lib/libsimgrid.so.3.12
teshsuite/smpi/compute/compute3: teshsuite/smpi/compute/CMakeFiles/compute3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable compute3"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/compute && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/compute3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/smpi/compute/CMakeFiles/compute3.dir/build: teshsuite/smpi/compute/compute3
.PHONY : teshsuite/smpi/compute/CMakeFiles/compute3.dir/build

teshsuite/smpi/compute/CMakeFiles/compute3.dir/requires: teshsuite/smpi/compute/CMakeFiles/compute3.dir/compute3.c.o.requires
.PHONY : teshsuite/smpi/compute/CMakeFiles/compute3.dir/requires

teshsuite/smpi/compute/CMakeFiles/compute3.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/compute && $(CMAKE_COMMAND) -P CMakeFiles/compute3.dir/cmake_clean.cmake
.PHONY : teshsuite/smpi/compute/CMakeFiles/compute3.dir/clean

teshsuite/smpi/compute/CMakeFiles/compute3.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/compute /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/compute /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/compute/CMakeFiles/compute3.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/smpi/compute/CMakeFiles/compute3.dir/depend

