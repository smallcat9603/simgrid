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
include teshsuite/simdag/basic/CMakeFiles/basic6.dir/depend.make

# Include the progress variables for this target.
include teshsuite/simdag/basic/CMakeFiles/basic6.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/simdag/basic/CMakeFiles/basic6.dir/flags.make

teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o: teshsuite/simdag/basic/CMakeFiles/basic6.dir/flags.make
teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o: teshsuite/simdag/basic/basic6.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/basic && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/basic6.dir/basic6.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/basic/basic6.c

teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/basic6.dir/basic6.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/basic && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/basic/basic6.c > CMakeFiles/basic6.dir/basic6.c.i

teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/basic6.dir/basic6.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/basic && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/basic/basic6.c -o CMakeFiles/basic6.dir/basic6.c.s

teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o.requires:
.PHONY : teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o.requires

teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o.provides: teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o.requires
	$(MAKE) -f teshsuite/simdag/basic/CMakeFiles/basic6.dir/build.make teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o.provides.build
.PHONY : teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o.provides

teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o.provides.build: teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o

# Object files for target basic6
basic6_OBJECTS = \
"CMakeFiles/basic6.dir/basic6.c.o"

# External object files for target basic6
basic6_EXTERNAL_OBJECTS =

teshsuite/simdag/basic/basic6: teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o
teshsuite/simdag/basic/basic6: teshsuite/simdag/basic/CMakeFiles/basic6.dir/build.make
teshsuite/simdag/basic/basic6: lib/libsimgrid.so.3.12
teshsuite/simdag/basic/basic6: teshsuite/simdag/basic/CMakeFiles/basic6.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable basic6"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/basic && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/basic6.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/simdag/basic/CMakeFiles/basic6.dir/build: teshsuite/simdag/basic/basic6
.PHONY : teshsuite/simdag/basic/CMakeFiles/basic6.dir/build

teshsuite/simdag/basic/CMakeFiles/basic6.dir/requires: teshsuite/simdag/basic/CMakeFiles/basic6.dir/basic6.c.o.requires
.PHONY : teshsuite/simdag/basic/CMakeFiles/basic6.dir/requires

teshsuite/simdag/basic/CMakeFiles/basic6.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/basic && $(CMAKE_COMMAND) -P CMakeFiles/basic6.dir/cmake_clean.cmake
.PHONY : teshsuite/simdag/basic/CMakeFiles/basic6.dir/clean

teshsuite/simdag/basic/CMakeFiles/basic6.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/basic /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/basic /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/basic/CMakeFiles/basic6.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/simdag/basic/CMakeFiles/basic6.dir/depend

