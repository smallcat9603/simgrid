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
include examples/msg/io/CMakeFiles/file_unlink.dir/depend.make

# Include the progress variables for this target.
include examples/msg/io/CMakeFiles/file_unlink.dir/progress.make

# Include the compile flags for this target's objects.
include examples/msg/io/CMakeFiles/file_unlink.dir/flags.make

examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o: examples/msg/io/CMakeFiles/file_unlink.dir/flags.make
examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o: examples/msg/io/file_unlink.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/io && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/file_unlink.dir/file_unlink.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/io/file_unlink.c

examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/file_unlink.dir/file_unlink.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/io && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/io/file_unlink.c > CMakeFiles/file_unlink.dir/file_unlink.c.i

examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/file_unlink.dir/file_unlink.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/io && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/io/file_unlink.c -o CMakeFiles/file_unlink.dir/file_unlink.c.s

examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o.requires:
.PHONY : examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o.requires

examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o.provides: examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o.requires
	$(MAKE) -f examples/msg/io/CMakeFiles/file_unlink.dir/build.make examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o.provides.build
.PHONY : examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o.provides

examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o.provides.build: examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o

# Object files for target file_unlink
file_unlink_OBJECTS = \
"CMakeFiles/file_unlink.dir/file_unlink.c.o"

# External object files for target file_unlink
file_unlink_EXTERNAL_OBJECTS =

examples/msg/io/file_unlink: examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o
examples/msg/io/file_unlink: examples/msg/io/CMakeFiles/file_unlink.dir/build.make
examples/msg/io/file_unlink: lib/libsimgrid.so.3.12
examples/msg/io/file_unlink: examples/msg/io/CMakeFiles/file_unlink.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable file_unlink"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/io && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/file_unlink.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/msg/io/CMakeFiles/file_unlink.dir/build: examples/msg/io/file_unlink
.PHONY : examples/msg/io/CMakeFiles/file_unlink.dir/build

examples/msg/io/CMakeFiles/file_unlink.dir/requires: examples/msg/io/CMakeFiles/file_unlink.dir/file_unlink.c.o.requires
.PHONY : examples/msg/io/CMakeFiles/file_unlink.dir/requires

examples/msg/io/CMakeFiles/file_unlink.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/io && $(CMAKE_COMMAND) -P CMakeFiles/file_unlink.dir/cmake_clean.cmake
.PHONY : examples/msg/io/CMakeFiles/file_unlink.dir/clean

examples/msg/io/CMakeFiles/file_unlink.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/io /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/io /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/examples/msg/io/CMakeFiles/file_unlink.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/msg/io/CMakeFiles/file_unlink.dir/depend

