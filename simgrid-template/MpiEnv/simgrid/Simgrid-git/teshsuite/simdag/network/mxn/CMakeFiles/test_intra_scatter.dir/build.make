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
include teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/depend.make

# Include the progress variables for this target.
include teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/flags.make

teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o: teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/flags.make
teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o: teshsuite/simdag/network/mxn/test_intra_scatter.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/mxn && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/mxn/test_intra_scatter.c

teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/mxn && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/mxn/test_intra_scatter.c > CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.i

teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/mxn && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/mxn/test_intra_scatter.c -o CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.s

teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o.requires:
.PHONY : teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o.requires

teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o.provides: teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o.requires
	$(MAKE) -f teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/build.make teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o.provides.build
.PHONY : teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o.provides

teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o.provides.build: teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o

# Object files for target test_intra_scatter
test_intra_scatter_OBJECTS = \
"CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o"

# External object files for target test_intra_scatter
test_intra_scatter_EXTERNAL_OBJECTS =

teshsuite/simdag/network/mxn/test_intra_scatter: teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o
teshsuite/simdag/network/mxn/test_intra_scatter: teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/build.make
teshsuite/simdag/network/mxn/test_intra_scatter: lib/libsimgrid.so.3.12
teshsuite/simdag/network/mxn/test_intra_scatter: teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable test_intra_scatter"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/mxn && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_intra_scatter.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/build: teshsuite/simdag/network/mxn/test_intra_scatter
.PHONY : teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/build

teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/requires: teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/test_intra_scatter.c.o.requires
.PHONY : teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/requires

teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/mxn && $(CMAKE_COMMAND) -P CMakeFiles/test_intra_scatter.dir/cmake_clean.cmake
.PHONY : teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/clean

teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/mxn /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/mxn /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/simdag/network/mxn/CMakeFiles/test_intra_scatter.dir/depend

