# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

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
CMAKE_SOURCE_DIR = /source/tutorial

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /source/tutorial

# Include any dependencies generated for this target.
include CMakeFiles/ping-pong.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ping-pong.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ping-pong.dir/flags.make

CMakeFiles/ping-pong.dir/ping-pong.cpp.o: CMakeFiles/ping-pong.dir/flags.make
CMakeFiles/ping-pong.dir/ping-pong.cpp.o: ping-pong.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/source/tutorial/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ping-pong.dir/ping-pong.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ping-pong.dir/ping-pong.cpp.o -c /source/tutorial/ping-pong.cpp

CMakeFiles/ping-pong.dir/ping-pong.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ping-pong.dir/ping-pong.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /source/tutorial/ping-pong.cpp > CMakeFiles/ping-pong.dir/ping-pong.cpp.i

CMakeFiles/ping-pong.dir/ping-pong.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ping-pong.dir/ping-pong.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /source/tutorial/ping-pong.cpp -o CMakeFiles/ping-pong.dir/ping-pong.cpp.s

# Object files for target ping-pong
ping__pong_OBJECTS = \
"CMakeFiles/ping-pong.dir/ping-pong.cpp.o"

# External object files for target ping-pong
ping__pong_EXTERNAL_OBJECTS =

ping-pong: CMakeFiles/ping-pong.dir/ping-pong.cpp.o
ping-pong: CMakeFiles/ping-pong.dir/build.make
ping-pong: /usr/lib/libsimgrid.so
ping-pong: CMakeFiles/ping-pong.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/source/tutorial/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ping-pong"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ping-pong.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ping-pong.dir/build: ping-pong

.PHONY : CMakeFiles/ping-pong.dir/build

CMakeFiles/ping-pong.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ping-pong.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ping-pong.dir/clean

CMakeFiles/ping-pong.dir/depend:
	cd /source/tutorial && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /source/tutorial /source/tutorial /source/tutorial /source/tutorial /source/tutorial/CMakeFiles/ping-pong.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ping-pong.dir/depend

