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
include teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/depend.make

# Include the progress variables for this target.
include teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/progress.make

# Include the compile flags for this target's objects.
include teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/flags.make

teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o: teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/flags.make
teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o: teshsuite/simdag/partask/test_comp_only_par.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/partask && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o   -c /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/partask/test_comp_only_par.c

teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.i"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/partask && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/partask/test_comp_only_par.c > CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.i

teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.s"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/partask && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/partask/test_comp_only_par.c -o CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.s

teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o.requires:
.PHONY : teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o.requires

teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o.provides: teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o.requires
	$(MAKE) -f teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/build.make teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o.provides.build
.PHONY : teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o.provides

teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o.provides.build: teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o

# Object files for target test_comp_only_par
test_comp_only_par_OBJECTS = \
"CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o"

# External object files for target test_comp_only_par
test_comp_only_par_EXTERNAL_OBJECTS =

teshsuite/simdag/partask/test_comp_only_par: teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o
teshsuite/simdag/partask/test_comp_only_par: teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/build.make
teshsuite/simdag/partask/test_comp_only_par: lib/libsimgrid.so.3.12
teshsuite/simdag/partask/test_comp_only_par: teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable test_comp_only_par"
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/partask && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_comp_only_par.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/build: teshsuite/simdag/partask/test_comp_only_par
.PHONY : teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/build

teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/requires: teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/test_comp_only_par.c.o.requires
.PHONY : teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/requires

teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/clean:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/partask && $(CMAKE_COMMAND) -P CMakeFiles/test_comp_only_par.dir/cmake_clean.cmake
.PHONY : teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/clean

teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/depend:
	cd /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/partask /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/partask /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git/teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : teshsuite/simdag/partask/CMakeFiles/test_comp_only_par.dir/depend

