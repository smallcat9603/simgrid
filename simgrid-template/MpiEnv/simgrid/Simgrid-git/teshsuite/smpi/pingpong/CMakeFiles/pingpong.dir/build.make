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
include teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/depend.make

# Include the progress variables for this target.
include teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/flags.make

teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o: teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/flags.make
teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o: teshsuite/smpi/pingpong/pingpong.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/pingpong && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/pingpong.dir/pingpong.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/pingpong/pingpong.c

teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pingpong.dir/pingpong.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/pingpong && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/pingpong/pingpong.c > CMakeFiles/pingpong.dir/pingpong.c.i

teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pingpong.dir/pingpong.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/pingpong && /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/smpi_script/bin/smpicc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/pingpong/pingpong.c -o CMakeFiles/pingpong.dir/pingpong.c.s

teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o.requires:
.PHONY : teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o.requires

teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o.provides: teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o.requires
	$(MAKE) -f teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/build.make teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o.provides.build
.PHONY : teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o.provides

teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o.provides.build: teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o

# Object files for target pingpong
pingpong_OBJECTS = \
"CMakeFiles/pingpong.dir/pingpong.c.o"

# External object files for target pingpong
pingpong_EXTERNAL_OBJECTS =

teshsuite/smpi/pingpong/pingpong: teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o
teshsuite/smpi/pingpong/pingpong: teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/build.make
teshsuite/smpi/pingpong/pingpong: lib/libsimgrid.so.3.12
teshsuite/smpi/pingpong/pingpong: teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable pingpong"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/pingpong && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/pingpong.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/build: teshsuite/smpi/pingpong/pingpong
.PHONY : teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/build

teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/requires: teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/pingpong.c.o.requires
.PHONY : teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/requires

teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/pingpong && $(CMAKE_COMMAND) -P CMakeFiles/pingpong.dir/cmake_clean.cmake
.PHONY : teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/clean

teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/pingpong /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/pingpong /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/smpi/pingpong/CMakeFiles/pingpong.dir/depend
