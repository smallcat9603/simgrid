# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_SOURCE_DIR = /github/simgrid/simgrid-3.26

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /github/simgrid/simgrid-3.26/build

# Include any dependencies generated for this target.
include teshsuite/smpi/CMakeFiles/io-shared.dir/depend.make

# Include the progress variables for this target.
include teshsuite/smpi/CMakeFiles/io-shared.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/smpi/CMakeFiles/io-shared.dir/flags.make

teshsuite/smpi/CMakeFiles/io-shared.dir/io-shared/io-shared.c.o: teshsuite/smpi/CMakeFiles/io-shared.dir/flags.make
teshsuite/smpi/CMakeFiles/io-shared.dir/io-shared/io-shared.c.o: ../teshsuite/smpi/io-shared/io-shared.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object teshsuite/smpi/CMakeFiles/io-shared.dir/io-shared/io-shared.c.o"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && /github/simgrid/simgrid-3.26/build/smpi_script/bin/smpicc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/io-shared.dir/io-shared/io-shared.c.o   -c /github/simgrid/simgrid-3.26/teshsuite/smpi/io-shared/io-shared.c

teshsuite/smpi/CMakeFiles/io-shared.dir/io-shared/io-shared.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/io-shared.dir/io-shared/io-shared.c.i"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && /github/simgrid/simgrid-3.26/build/smpi_script/bin/smpicc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/teshsuite/smpi/io-shared/io-shared.c > CMakeFiles/io-shared.dir/io-shared/io-shared.c.i

teshsuite/smpi/CMakeFiles/io-shared.dir/io-shared/io-shared.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/io-shared.dir/io-shared/io-shared.c.s"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && /github/simgrid/simgrid-3.26/build/smpi_script/bin/smpicc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/teshsuite/smpi/io-shared/io-shared.c -o CMakeFiles/io-shared.dir/io-shared/io-shared.c.s

# Object files for target io-shared
io__shared_OBJECTS = \
"CMakeFiles/io-shared.dir/io-shared/io-shared.c.o"

# External object files for target io-shared
io__shared_EXTERNAL_OBJECTS =

teshsuite/smpi/io-shared/io-shared: teshsuite/smpi/CMakeFiles/io-shared.dir/io-shared/io-shared.c.o
teshsuite/smpi/io-shared/io-shared: teshsuite/smpi/CMakeFiles/io-shared.dir/build.make
teshsuite/smpi/io-shared/io-shared: lib/libsimgrid.so.3.26
teshsuite/smpi/io-shared/io-shared: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
teshsuite/smpi/io-shared/io-shared: teshsuite/smpi/CMakeFiles/io-shared.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable io-shared/io-shared"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/io-shared.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/smpi/CMakeFiles/io-shared.dir/build: teshsuite/smpi/io-shared/io-shared

.PHONY : teshsuite/smpi/CMakeFiles/io-shared.dir/build

teshsuite/smpi/CMakeFiles/io-shared.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && $(CMAKE_COMMAND) -P CMakeFiles/io-shared.dir/cmake_clean.cmake
.PHONY : teshsuite/smpi/CMakeFiles/io-shared.dir/clean

teshsuite/smpi/CMakeFiles/io-shared.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/teshsuite/smpi /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/teshsuite/smpi /github/simgrid/simgrid-3.26/build/teshsuite/smpi/CMakeFiles/io-shared.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/smpi/CMakeFiles/io-shared.dir/depend

