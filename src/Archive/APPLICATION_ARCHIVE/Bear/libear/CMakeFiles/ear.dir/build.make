# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.2

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
CMAKE_SOURCE_DIR = /home/kisp/Bear_src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kisp/Bear

# Include any dependencies generated for this target.
include libear/CMakeFiles/ear.dir/depend.make

# Include the progress variables for this target.
include libear/CMakeFiles/ear.dir/progress.make

# Include the compile flags for this target's objects.
include libear/CMakeFiles/ear.dir/flags.make

libear/CMakeFiles/ear.dir/ear.c.o: libear/CMakeFiles/ear.dir/flags.make
libear/CMakeFiles/ear.dir/ear.c.o: /home/kisp/Bear_src/libear/ear.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/kisp/Bear/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object libear/CMakeFiles/ear.dir/ear.c.o"
	cd /home/kisp/Bear/libear && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/ear.dir/ear.c.o   -c /home/kisp/Bear_src/libear/ear.c

libear/CMakeFiles/ear.dir/ear.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ear.dir/ear.c.i"
	cd /home/kisp/Bear/libear && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/kisp/Bear_src/libear/ear.c > CMakeFiles/ear.dir/ear.c.i

libear/CMakeFiles/ear.dir/ear.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ear.dir/ear.c.s"
	cd /home/kisp/Bear/libear && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/kisp/Bear_src/libear/ear.c -o CMakeFiles/ear.dir/ear.c.s

libear/CMakeFiles/ear.dir/ear.c.o.requires:
.PHONY : libear/CMakeFiles/ear.dir/ear.c.o.requires

libear/CMakeFiles/ear.dir/ear.c.o.provides: libear/CMakeFiles/ear.dir/ear.c.o.requires
	$(MAKE) -f libear/CMakeFiles/ear.dir/build.make libear/CMakeFiles/ear.dir/ear.c.o.provides.build
.PHONY : libear/CMakeFiles/ear.dir/ear.c.o.provides

libear/CMakeFiles/ear.dir/ear.c.o.provides.build: libear/CMakeFiles/ear.dir/ear.c.o

# Object files for target ear
ear_OBJECTS = \
"CMakeFiles/ear.dir/ear.c.o"

# External object files for target ear
ear_EXTERNAL_OBJECTS =

libear/libear.so: libear/CMakeFiles/ear.dir/ear.c.o
libear/libear.so: libear/CMakeFiles/ear.dir/build.make
libear/libear.so: libear/CMakeFiles/ear.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C shared library libear.so"
	cd /home/kisp/Bear/libear && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ear.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
libear/CMakeFiles/ear.dir/build: libear/libear.so
.PHONY : libear/CMakeFiles/ear.dir/build

libear/CMakeFiles/ear.dir/requires: libear/CMakeFiles/ear.dir/ear.c.o.requires
.PHONY : libear/CMakeFiles/ear.dir/requires

libear/CMakeFiles/ear.dir/clean:
	cd /home/kisp/Bear/libear && $(CMAKE_COMMAND) -P CMakeFiles/ear.dir/cmake_clean.cmake
.PHONY : libear/CMakeFiles/ear.dir/clean

libear/CMakeFiles/ear.dir/depend:
	cd /home/kisp/Bear && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kisp/Bear_src /home/kisp/Bear_src/libear /home/kisp/Bear /home/kisp/Bear/libear /home/kisp/Bear/libear/CMakeFiles/ear.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : libear/CMakeFiles/ear.dir/depend
