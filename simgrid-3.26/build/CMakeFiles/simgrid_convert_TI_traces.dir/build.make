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

# Utility rule file for simgrid_convert_TI_traces.

# Include the progress variables for this target.
include CMakeFiles/simgrid_convert_TI_traces.dir/progress.make

CMakeFiles/simgrid_convert_TI_traces:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Install /github/simgrid/simgrid-3.26/build/bin/simgrid_convert_TI_traces"
	/usr/bin/cmake -E copy /github/simgrid/simgrid-3.26/tools/simgrid_convert_TI_traces.py /github/simgrid/simgrid-3.26/build/bin/simgrid_convert_TI_traces

simgrid_convert_TI_traces: CMakeFiles/simgrid_convert_TI_traces
simgrid_convert_TI_traces: CMakeFiles/simgrid_convert_TI_traces.dir/build.make

.PHONY : simgrid_convert_TI_traces

# Rule to build all files generated by this target.
CMakeFiles/simgrid_convert_TI_traces.dir/build: simgrid_convert_TI_traces

.PHONY : CMakeFiles/simgrid_convert_TI_traces.dir/build

CMakeFiles/simgrid_convert_TI_traces.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/simgrid_convert_TI_traces.dir/cmake_clean.cmake
.PHONY : CMakeFiles/simgrid_convert_TI_traces.dir/clean

CMakeFiles/simgrid_convert_TI_traces.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/CMakeFiles/simgrid_convert_TI_traces.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/simgrid_convert_TI_traces.dir/depend
