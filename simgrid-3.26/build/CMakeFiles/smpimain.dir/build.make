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
include CMakeFiles/smpimain.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/smpimain.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/smpimain.dir/flags.make

CMakeFiles/smpimain.dir/src/smpi/smpi_main.c.o: CMakeFiles/smpimain.dir/flags.make
CMakeFiles/smpimain.dir/src/smpi/smpi_main.c.o: ../src/smpi/smpi_main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/smpimain.dir/src/smpi/smpi_main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/smpimain.dir/src/smpi/smpi_main.c.o   -c /github/simgrid/simgrid-3.26/src/smpi/smpi_main.c

CMakeFiles/smpimain.dir/src/smpi/smpi_main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/smpimain.dir/src/smpi/smpi_main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/src/smpi/smpi_main.c > CMakeFiles/smpimain.dir/src/smpi/smpi_main.c.i

CMakeFiles/smpimain.dir/src/smpi/smpi_main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/smpimain.dir/src/smpi/smpi_main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/src/smpi/smpi_main.c -o CMakeFiles/smpimain.dir/src/smpi/smpi_main.c.s

# Object files for target smpimain
smpimain_OBJECTS = \
"CMakeFiles/smpimain.dir/src/smpi/smpi_main.c.o"

# External object files for target smpimain
smpimain_EXTERNAL_OBJECTS =

lib/simgrid/smpimain: CMakeFiles/smpimain.dir/src/smpi/smpi_main.c.o
lib/simgrid/smpimain: CMakeFiles/smpimain.dir/build.make
lib/simgrid/smpimain: lib/libsimgrid.so.3.26
lib/simgrid/smpimain: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
lib/simgrid/smpimain: CMakeFiles/smpimain.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable lib/simgrid/smpimain"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/smpimain.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/smpimain.dir/build: lib/simgrid/smpimain

.PHONY : CMakeFiles/smpimain.dir/build

CMakeFiles/smpimain.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/smpimain.dir/cmake_clean.cmake
.PHONY : CMakeFiles/smpimain.dir/clean

CMakeFiles/smpimain.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/CMakeFiles/smpimain.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/smpimain.dir/depend

