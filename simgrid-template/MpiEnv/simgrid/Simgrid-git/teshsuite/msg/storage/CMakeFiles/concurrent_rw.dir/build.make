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
include teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/depend.make

# Include the progress variables for this target.
include teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/flags.make

teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o: teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/flags.make
teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o: teshsuite/msg/storage/concurrent_rw.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/msg/storage && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/msg/storage/concurrent_rw.c

teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/concurrent_rw.dir/concurrent_rw.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/msg/storage && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/msg/storage/concurrent_rw.c > CMakeFiles/concurrent_rw.dir/concurrent_rw.c.i

teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/concurrent_rw.dir/concurrent_rw.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/msg/storage && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/msg/storage/concurrent_rw.c -o CMakeFiles/concurrent_rw.dir/concurrent_rw.c.s

teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o.requires:
.PHONY : teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o.requires

teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o.provides: teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o.requires
	$(MAKE) -f teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/build.make teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o.provides.build
.PHONY : teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o.provides

teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o.provides.build: teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o

# Object files for target concurrent_rw
concurrent_rw_OBJECTS = \
"CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o"

# External object files for target concurrent_rw
concurrent_rw_EXTERNAL_OBJECTS =

teshsuite/msg/storage/concurrent_rw: teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o
teshsuite/msg/storage/concurrent_rw: teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/build.make
teshsuite/msg/storage/concurrent_rw: lib/libsimgrid.so.3.12
teshsuite/msg/storage/concurrent_rw: teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable concurrent_rw"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/msg/storage && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/concurrent_rw.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/build: teshsuite/msg/storage/concurrent_rw
.PHONY : teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/build

teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/requires: teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/concurrent_rw.c.o.requires
.PHONY : teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/requires

teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/msg/storage && $(CMAKE_COMMAND) -P CMakeFiles/concurrent_rw.dir/cmake_clean.cmake
.PHONY : teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/clean

teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/msg/storage /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/msg/storage /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/msg/storage/CMakeFiles/concurrent_rw.dir/depend

