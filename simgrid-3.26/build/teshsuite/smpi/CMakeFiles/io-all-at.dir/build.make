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
include teshsuite/smpi/CMakeFiles/io-all-at.dir/depend.make

# Include the progress variables for this target.
include teshsuite/smpi/CMakeFiles/io-all-at.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/smpi/CMakeFiles/io-all-at.dir/flags.make

teshsuite/smpi/CMakeFiles/io-all-at.dir/io-all-at/io-all-at.c.o: teshsuite/smpi/CMakeFiles/io-all-at.dir/flags.make
teshsuite/smpi/CMakeFiles/io-all-at.dir/io-all-at/io-all-at.c.o: ../teshsuite/smpi/io-all-at/io-all-at.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object teshsuite/smpi/CMakeFiles/io-all-at.dir/io-all-at/io-all-at.c.o"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && /github/simgrid/simgrid-3.26/build/smpi_script/bin/smpicc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/io-all-at.dir/io-all-at/io-all-at.c.o   -c /github/simgrid/simgrid-3.26/teshsuite/smpi/io-all-at/io-all-at.c

teshsuite/smpi/CMakeFiles/io-all-at.dir/io-all-at/io-all-at.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/io-all-at.dir/io-all-at/io-all-at.c.i"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && /github/simgrid/simgrid-3.26/build/smpi_script/bin/smpicc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/teshsuite/smpi/io-all-at/io-all-at.c > CMakeFiles/io-all-at.dir/io-all-at/io-all-at.c.i

teshsuite/smpi/CMakeFiles/io-all-at.dir/io-all-at/io-all-at.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/io-all-at.dir/io-all-at/io-all-at.c.s"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && /github/simgrid/simgrid-3.26/build/smpi_script/bin/smpicc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/teshsuite/smpi/io-all-at/io-all-at.c -o CMakeFiles/io-all-at.dir/io-all-at/io-all-at.c.s

# Object files for target io-all-at
io__all__at_OBJECTS = \
"CMakeFiles/io-all-at.dir/io-all-at/io-all-at.c.o"

# External object files for target io-all-at
io__all__at_EXTERNAL_OBJECTS =

teshsuite/smpi/io-all-at/io-all-at: teshsuite/smpi/CMakeFiles/io-all-at.dir/io-all-at/io-all-at.c.o
teshsuite/smpi/io-all-at/io-all-at: teshsuite/smpi/CMakeFiles/io-all-at.dir/build.make
teshsuite/smpi/io-all-at/io-all-at: lib/libsimgrid.so.3.26
teshsuite/smpi/io-all-at/io-all-at: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
teshsuite/smpi/io-all-at/io-all-at: teshsuite/smpi/CMakeFiles/io-all-at.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable io-all-at/io-all-at"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/io-all-at.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/smpi/CMakeFiles/io-all-at.dir/build: teshsuite/smpi/io-all-at/io-all-at

.PHONY : teshsuite/smpi/CMakeFiles/io-all-at.dir/build

teshsuite/smpi/CMakeFiles/io-all-at.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && $(CMAKE_COMMAND) -P CMakeFiles/io-all-at.dir/cmake_clean.cmake
.PHONY : teshsuite/smpi/CMakeFiles/io-all-at.dir/clean

teshsuite/smpi/CMakeFiles/io-all-at.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/teshsuite/smpi /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/teshsuite/smpi /github/simgrid/simgrid-3.26/build/teshsuite/smpi/CMakeFiles/io-all-at.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/smpi/CMakeFiles/io-all-at.dir/depend

