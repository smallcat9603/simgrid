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
include teshsuite/smpi/vector/CMakeFiles/vector_test.dir/depend.make

# Include the progress variables for this target.
include teshsuite/smpi/vector/CMakeFiles/vector_test.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/smpi/vector/CMakeFiles/vector_test.dir/flags.make

teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o: teshsuite/smpi/vector/CMakeFiles/vector_test.dir/flags.make
teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o: teshsuite/smpi/vector/vector_test.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/vector && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/vector_test.dir/vector_test.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/vector/vector_test.c

teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/vector_test.dir/vector_test.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/vector && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/vector/vector_test.c > CMakeFiles/vector_test.dir/vector_test.c.i

teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/vector_test.dir/vector_test.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/vector && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/vector/vector_test.c -o CMakeFiles/vector_test.dir/vector_test.c.s

teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o.requires:
.PHONY : teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o.requires

teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o.provides: teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o.requires
	$(MAKE) -f teshsuite/smpi/vector/CMakeFiles/vector_test.dir/build.make teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o.provides.build
.PHONY : teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o.provides

teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o.provides.build: teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o

# Object files for target vector_test
vector_test_OBJECTS = \
"CMakeFiles/vector_test.dir/vector_test.c.o"

# External object files for target vector_test
vector_test_EXTERNAL_OBJECTS =

teshsuite/smpi/vector/vector_test: teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o
teshsuite/smpi/vector/vector_test: teshsuite/smpi/vector/CMakeFiles/vector_test.dir/build.make
teshsuite/smpi/vector/vector_test: lib/libsimgrid.so.3.12
teshsuite/smpi/vector/vector_test: teshsuite/smpi/vector/CMakeFiles/vector_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable vector_test"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/vector && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/vector_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/smpi/vector/CMakeFiles/vector_test.dir/build: teshsuite/smpi/vector/vector_test
.PHONY : teshsuite/smpi/vector/CMakeFiles/vector_test.dir/build

teshsuite/smpi/vector/CMakeFiles/vector_test.dir/requires: teshsuite/smpi/vector/CMakeFiles/vector_test.dir/vector_test.c.o.requires
.PHONY : teshsuite/smpi/vector/CMakeFiles/vector_test.dir/requires

teshsuite/smpi/vector/CMakeFiles/vector_test.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/vector && $(CMAKE_COMMAND) -P CMakeFiles/vector_test.dir/cmake_clean.cmake
.PHONY : teshsuite/smpi/vector/CMakeFiles/vector_test.dir/clean

teshsuite/smpi/vector/CMakeFiles/vector_test.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/vector /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/vector /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/vector/CMakeFiles/vector_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/smpi/vector/CMakeFiles/vector_test.dir/depend

