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

# Utility rule file for documentation.

# Include the progress variables for this target.
include CMakeFiles/documentation.dir/progress.make

documentation: CMakeFiles/documentation.dir/build.make

.PHONY : documentation

# Rule to build all files generated by this target.
CMakeFiles/documentation.dir/build: documentation

.PHONY : CMakeFiles/documentation.dir/build

CMakeFiles/documentation.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/documentation.dir/cmake_clean.cmake
.PHONY : CMakeFiles/documentation.dir/clean

CMakeFiles/documentation.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/CMakeFiles/documentation.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/documentation.dir/depend

