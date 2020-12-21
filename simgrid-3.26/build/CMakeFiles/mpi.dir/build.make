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
include CMakeFiles/mpi.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/mpi.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/mpi.dir/flags.make

CMakeFiles/mpi.dir/src/smpi/mpif.f90.o: CMakeFiles/mpi.dir/flags.make
CMakeFiles/mpi.dir/src/smpi/mpif.f90.o: src/smpi/mpif.f90
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building Fortran object CMakeFiles/mpi.dir/src/smpi/mpif.f90.o"
	/usr/bin/gfortran $(Fortran_DEFINES) $(Fortran_INCLUDES) $(Fortran_FLAGS) -c /github/simgrid/simgrid-3.26/build/src/smpi/mpif.f90 -o CMakeFiles/mpi.dir/src/smpi/mpif.f90.o

CMakeFiles/mpi.dir/src/smpi/mpif.f90.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing Fortran source to CMakeFiles/mpi.dir/src/smpi/mpif.f90.i"
	/usr/bin/gfortran $(Fortran_DEFINES) $(Fortran_INCLUDES) $(Fortran_FLAGS) -E /github/simgrid/simgrid-3.26/build/src/smpi/mpif.f90 > CMakeFiles/mpi.dir/src/smpi/mpif.f90.i

CMakeFiles/mpi.dir/src/smpi/mpif.f90.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling Fortran source to assembly CMakeFiles/mpi.dir/src/smpi/mpif.f90.s"
	/usr/bin/gfortran $(Fortran_DEFINES) $(Fortran_INCLUDES) $(Fortran_FLAGS) -S /github/simgrid/simgrid-3.26/build/src/smpi/mpif.f90 -o CMakeFiles/mpi.dir/src/smpi/mpif.f90.s

# Object files for target mpi
mpi_OBJECTS = \
"CMakeFiles/mpi.dir/src/smpi/mpif.f90.o"

# External object files for target mpi
mpi_EXTERNAL_OBJECTS =

lib/libmpi.so: CMakeFiles/mpi.dir/src/smpi/mpif.f90.o
lib/libmpi.so: CMakeFiles/mpi.dir/build.make
lib/libmpi.so: CMakeFiles/mpi.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking Fortran shared library lib/libmpi.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mpi.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/mpi.dir/build: lib/libmpi.so

.PHONY : CMakeFiles/mpi.dir/build

CMakeFiles/mpi.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/mpi.dir/cmake_clean.cmake
.PHONY : CMakeFiles/mpi.dir/clean

CMakeFiles/mpi.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/CMakeFiles/mpi.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/mpi.dir/depend

