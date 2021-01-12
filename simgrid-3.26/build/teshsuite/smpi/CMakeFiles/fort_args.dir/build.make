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
include teshsuite/smpi/CMakeFiles/fort_args.dir/depend.make

# Include the progress variables for this target.
include teshsuite/smpi/CMakeFiles/fort_args.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/smpi/CMakeFiles/fort_args.dir/flags.make

teshsuite/smpi/CMakeFiles/fort_args.dir/fort_args/fort_args.f90.o: teshsuite/smpi/CMakeFiles/fort_args.dir/flags.make
teshsuite/smpi/CMakeFiles/fort_args.dir/fort_args/fort_args.f90.o: ../teshsuite/smpi/fort_args/fort_args.f90
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building Fortran object teshsuite/smpi/CMakeFiles/fort_args.dir/fort_args/fort_args.f90.o"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && /github/simgrid/simgrid-3.26/build/smpi_script/bin/smpif90 $(Fortran_DEFINES) $(Fortran_INCLUDES) $(Fortran_FLAGS) -c /github/simgrid/simgrid-3.26/teshsuite/smpi/fort_args/fort_args.f90 -o CMakeFiles/fort_args.dir/fort_args/fort_args.f90.o

teshsuite/smpi/CMakeFiles/fort_args.dir/fort_args/fort_args.f90.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing Fortran source to CMakeFiles/fort_args.dir/fort_args/fort_args.f90.i"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && /github/simgrid/simgrid-3.26/build/smpi_script/bin/smpif90 $(Fortran_DEFINES) $(Fortran_INCLUDES) $(Fortran_FLAGS) -E /github/simgrid/simgrid-3.26/teshsuite/smpi/fort_args/fort_args.f90 > CMakeFiles/fort_args.dir/fort_args/fort_args.f90.i

teshsuite/smpi/CMakeFiles/fort_args.dir/fort_args/fort_args.f90.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling Fortran source to assembly CMakeFiles/fort_args.dir/fort_args/fort_args.f90.s"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && /github/simgrid/simgrid-3.26/build/smpi_script/bin/smpif90 $(Fortran_DEFINES) $(Fortran_INCLUDES) $(Fortran_FLAGS) -S /github/simgrid/simgrid-3.26/teshsuite/smpi/fort_args/fort_args.f90 -o CMakeFiles/fort_args.dir/fort_args/fort_args.f90.s

# Object files for target fort_args
fort_args_OBJECTS = \
"CMakeFiles/fort_args.dir/fort_args/fort_args.f90.o"

# External object files for target fort_args
fort_args_EXTERNAL_OBJECTS =

teshsuite/smpi/fort_args/fort_args: teshsuite/smpi/CMakeFiles/fort_args.dir/fort_args/fort_args.f90.o
teshsuite/smpi/fort_args/fort_args: teshsuite/smpi/CMakeFiles/fort_args.dir/build.make
teshsuite/smpi/fort_args/fort_args: lib/libsimgrid.so.3.26
teshsuite/smpi/fort_args/fort_args: lib/libmpi.so
teshsuite/smpi/fort_args/fort_args: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
teshsuite/smpi/fort_args/fort_args: teshsuite/smpi/CMakeFiles/fort_args.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking Fortran executable fort_args/fort_args"
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/fort_args.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/smpi/CMakeFiles/fort_args.dir/build: teshsuite/smpi/fort_args/fort_args

.PHONY : teshsuite/smpi/CMakeFiles/fort_args.dir/build

teshsuite/smpi/CMakeFiles/fort_args.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/teshsuite/smpi && $(CMAKE_COMMAND) -P CMakeFiles/fort_args.dir/cmake_clean.cmake
.PHONY : teshsuite/smpi/CMakeFiles/fort_args.dir/clean

teshsuite/smpi/CMakeFiles/fort_args.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/teshsuite/smpi /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/teshsuite/smpi /github/simgrid/simgrid-3.26/build/teshsuite/smpi/CMakeFiles/fort_args.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/smpi/CMakeFiles/fort_args.dir/depend
