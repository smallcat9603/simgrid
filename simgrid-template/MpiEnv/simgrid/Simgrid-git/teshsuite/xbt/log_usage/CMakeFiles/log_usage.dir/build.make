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
include teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/depend.make

# Include the progress variables for this target.
include teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/flags.make

teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o: teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/flags.make
teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o: teshsuite/xbt/log_usage/log_usage.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/xbt/log_usage && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/log_usage.dir/log_usage.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/xbt/log_usage/log_usage.c

teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/log_usage.dir/log_usage.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/xbt/log_usage && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/xbt/log_usage/log_usage.c > CMakeFiles/log_usage.dir/log_usage.c.i

teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/log_usage.dir/log_usage.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/xbt/log_usage && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/xbt/log_usage/log_usage.c -o CMakeFiles/log_usage.dir/log_usage.c.s

teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o.requires:
.PHONY : teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o.requires

teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o.provides: teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o.requires
	$(MAKE) -f teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/build.make teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o.provides.build
.PHONY : teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o.provides

teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o.provides.build: teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o

# Object files for target log_usage
log_usage_OBJECTS = \
"CMakeFiles/log_usage.dir/log_usage.c.o"

# External object files for target log_usage
log_usage_EXTERNAL_OBJECTS =

teshsuite/xbt/log_usage/log_usage: teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o
teshsuite/xbt/log_usage/log_usage: teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/build.make
teshsuite/xbt/log_usage/log_usage: lib/libsimgrid.so.3.12
teshsuite/xbt/log_usage/log_usage: teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable log_usage"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/xbt/log_usage && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/log_usage.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/build: teshsuite/xbt/log_usage/log_usage
.PHONY : teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/build

teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/requires: teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/log_usage.c.o.requires
.PHONY : teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/requires

teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/xbt/log_usage && $(CMAKE_COMMAND) -P CMakeFiles/log_usage.dir/cmake_clean.cmake
.PHONY : teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/clean

teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/xbt/log_usage /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/xbt/log_usage /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/xbt/log_usage/CMakeFiles/log_usage.dir/depend
