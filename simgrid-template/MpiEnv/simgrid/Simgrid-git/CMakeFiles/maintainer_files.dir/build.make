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

# Utility rule file for maintainer_files.

# Include the progress variables for this target.
include CMakeFiles/maintainer_files.dir/progress.make

CMakeFiles/maintainer_files: include/surf/simgrid_dtd.h
CMakeFiles/maintainer_files: include/xbt/graphxml.h
CMakeFiles/maintainer_files: src/simdag/dax_dtd.h
CMakeFiles/maintainer_files: src/surf/simgrid_dtd.c
CMakeFiles/maintainer_files: src/xbt/graphxml.c
CMakeFiles/maintainer_files: src/simdag/dax_dtd.c

maintainer_files: CMakeFiles/maintainer_files
maintainer_files: CMakeFiles/maintainer_files.dir/build.make
.PHONY : maintainer_files

# Rule to build all files generated by this target.
CMakeFiles/maintainer_files.dir/build: maintainer_files
.PHONY : CMakeFiles/maintainer_files.dir/build

CMakeFiles/maintainer_files.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/maintainer_files.dir/cmake_clean.cmake
.PHONY : CMakeFiles/maintainer_files.dir/clean

CMakeFiles/maintainer_files.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles/maintainer_files.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/maintainer_files.dir/depend
