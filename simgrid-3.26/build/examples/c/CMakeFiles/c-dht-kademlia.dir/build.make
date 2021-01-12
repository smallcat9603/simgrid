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
include examples/c/CMakeFiles/c-dht-kademlia.dir/depend.make

# Include the progress variables for this target.
include examples/c/CMakeFiles/c-dht-kademlia.dir/progress.make

# Include the compile flags for this target's objects.
include examples/c/CMakeFiles/c-dht-kademlia.dir/flags.make

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/dht-kademlia.c.o: examples/c/CMakeFiles/c-dht-kademlia.dir/flags.make
examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/dht-kademlia.c.o: ../examples/c/dht-kademlia/dht-kademlia.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/dht-kademlia.c.o"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/c-dht-kademlia.dir/dht-kademlia/dht-kademlia.c.o   -c /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/dht-kademlia.c

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/dht-kademlia.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/c-dht-kademlia.dir/dht-kademlia/dht-kademlia.c.i"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/dht-kademlia.c > CMakeFiles/c-dht-kademlia.dir/dht-kademlia/dht-kademlia.c.i

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/dht-kademlia.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/c-dht-kademlia.dir/dht-kademlia/dht-kademlia.c.s"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/dht-kademlia.c -o CMakeFiles/c-dht-kademlia.dir/dht-kademlia/dht-kademlia.c.s

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/node.c.o: examples/c/CMakeFiles/c-dht-kademlia.dir/flags.make
examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/node.c.o: ../examples/c/dht-kademlia/node.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/node.c.o"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/c-dht-kademlia.dir/dht-kademlia/node.c.o   -c /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/node.c

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/node.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/c-dht-kademlia.dir/dht-kademlia/node.c.i"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/node.c > CMakeFiles/c-dht-kademlia.dir/dht-kademlia/node.c.i

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/node.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/c-dht-kademlia.dir/dht-kademlia/node.c.s"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/node.c -o CMakeFiles/c-dht-kademlia.dir/dht-kademlia/node.c.s

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/routing_table.c.o: examples/c/CMakeFiles/c-dht-kademlia.dir/flags.make
examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/routing_table.c.o: ../examples/c/dht-kademlia/routing_table.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/routing_table.c.o"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/c-dht-kademlia.dir/dht-kademlia/routing_table.c.o   -c /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/routing_table.c

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/routing_table.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/c-dht-kademlia.dir/dht-kademlia/routing_table.c.i"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/routing_table.c > CMakeFiles/c-dht-kademlia.dir/dht-kademlia/routing_table.c.i

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/routing_table.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/c-dht-kademlia.dir/dht-kademlia/routing_table.c.s"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/routing_table.c -o CMakeFiles/c-dht-kademlia.dir/dht-kademlia/routing_table.c.s

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/message.c.o: examples/c/CMakeFiles/c-dht-kademlia.dir/flags.make
examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/message.c.o: ../examples/c/dht-kademlia/message.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/message.c.o"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/c-dht-kademlia.dir/dht-kademlia/message.c.o   -c /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/message.c

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/message.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/c-dht-kademlia.dir/dht-kademlia/message.c.i"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/message.c > CMakeFiles/c-dht-kademlia.dir/dht-kademlia/message.c.i

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/message.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/c-dht-kademlia.dir/dht-kademlia/message.c.s"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/message.c -o CMakeFiles/c-dht-kademlia.dir/dht-kademlia/message.c.s

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/answer.c.o: examples/c/CMakeFiles/c-dht-kademlia.dir/flags.make
examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/answer.c.o: ../examples/c/dht-kademlia/answer.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/answer.c.o"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/c-dht-kademlia.dir/dht-kademlia/answer.c.o   -c /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/answer.c

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/answer.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/c-dht-kademlia.dir/dht-kademlia/answer.c.i"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/answer.c > CMakeFiles/c-dht-kademlia.dir/dht-kademlia/answer.c.i

examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/answer.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/c-dht-kademlia.dir/dht-kademlia/answer.c.s"
	cd /github/simgrid/simgrid-3.26/build/examples/c && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /github/simgrid/simgrid-3.26/examples/c/dht-kademlia/answer.c -o CMakeFiles/c-dht-kademlia.dir/dht-kademlia/answer.c.s

# Object files for target c-dht-kademlia
c__dht__kademlia_OBJECTS = \
"CMakeFiles/c-dht-kademlia.dir/dht-kademlia/dht-kademlia.c.o" \
"CMakeFiles/c-dht-kademlia.dir/dht-kademlia/node.c.o" \
"CMakeFiles/c-dht-kademlia.dir/dht-kademlia/routing_table.c.o" \
"CMakeFiles/c-dht-kademlia.dir/dht-kademlia/message.c.o" \
"CMakeFiles/c-dht-kademlia.dir/dht-kademlia/answer.c.o"

# External object files for target c-dht-kademlia
c__dht__kademlia_EXTERNAL_OBJECTS =

examples/c/dht-kademlia/c-dht-kademlia: examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/dht-kademlia.c.o
examples/c/dht-kademlia/c-dht-kademlia: examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/node.c.o
examples/c/dht-kademlia/c-dht-kademlia: examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/routing_table.c.o
examples/c/dht-kademlia/c-dht-kademlia: examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/message.c.o
examples/c/dht-kademlia/c-dht-kademlia: examples/c/CMakeFiles/c-dht-kademlia.dir/dht-kademlia/answer.c.o
examples/c/dht-kademlia/c-dht-kademlia: examples/c/CMakeFiles/c-dht-kademlia.dir/build.make
examples/c/dht-kademlia/c-dht-kademlia: lib/libsimgrid.so.3.26
examples/c/dht-kademlia/c-dht-kademlia: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.71.0
examples/c/dht-kademlia/c-dht-kademlia: examples/c/CMakeFiles/c-dht-kademlia.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/github/simgrid/simgrid-3.26/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking C executable dht-kademlia/c-dht-kademlia"
	cd /github/simgrid/simgrid-3.26/build/examples/c && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/c-dht-kademlia.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/c/CMakeFiles/c-dht-kademlia.dir/build: examples/c/dht-kademlia/c-dht-kademlia

.PHONY : examples/c/CMakeFiles/c-dht-kademlia.dir/build

examples/c/CMakeFiles/c-dht-kademlia.dir/clean:
	cd /github/simgrid/simgrid-3.26/build/examples/c && $(CMAKE_COMMAND) -P CMakeFiles/c-dht-kademlia.dir/cmake_clean.cmake
.PHONY : examples/c/CMakeFiles/c-dht-kademlia.dir/clean

examples/c/CMakeFiles/c-dht-kademlia.dir/depend:
	cd /github/simgrid/simgrid-3.26/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /github/simgrid/simgrid-3.26 /github/simgrid/simgrid-3.26/examples/c /github/simgrid/simgrid-3.26/build /github/simgrid/simgrid-3.26/build/examples/c /github/simgrid/simgrid-3.26/build/examples/c/CMakeFiles/c-dht-kademlia.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/c/CMakeFiles/c-dht-kademlia.dir/depend
