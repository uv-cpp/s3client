# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.25.1/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.25.1/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/ugovaretto/projects/s3client

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/ugovaretto/projects/s3client/build

# Include any dependencies generated for this target.
include test/CMakeFiles/bucket-test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/bucket-test.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/bucket-test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/bucket-test.dir/flags.make

test/CMakeFiles/bucket-test.dir/api/bucket-test.cpp.o: test/CMakeFiles/bucket-test.dir/flags.make
test/CMakeFiles/bucket-test.dir/api/bucket-test.cpp.o: /Users/ugovaretto/projects/s3client/test/api/bucket-test.cpp
test/CMakeFiles/bucket-test.dir/api/bucket-test.cpp.o: test/CMakeFiles/bucket-test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/ugovaretto/projects/s3client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/bucket-test.dir/api/bucket-test.cpp.o"
	cd /Users/ugovaretto/projects/s3client/build/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/bucket-test.dir/api/bucket-test.cpp.o -MF CMakeFiles/bucket-test.dir/api/bucket-test.cpp.o.d -o CMakeFiles/bucket-test.dir/api/bucket-test.cpp.o -c /Users/ugovaretto/projects/s3client/test/api/bucket-test.cpp

test/CMakeFiles/bucket-test.dir/api/bucket-test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bucket-test.dir/api/bucket-test.cpp.i"
	cd /Users/ugovaretto/projects/s3client/build/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/ugovaretto/projects/s3client/test/api/bucket-test.cpp > CMakeFiles/bucket-test.dir/api/bucket-test.cpp.i

test/CMakeFiles/bucket-test.dir/api/bucket-test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bucket-test.dir/api/bucket-test.cpp.s"
	cd /Users/ugovaretto/projects/s3client/build/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/ugovaretto/projects/s3client/test/api/bucket-test.cpp -o CMakeFiles/bucket-test.dir/api/bucket-test.cpp.s

test/CMakeFiles/bucket-test.dir/utility.cpp.o: test/CMakeFiles/bucket-test.dir/flags.make
test/CMakeFiles/bucket-test.dir/utility.cpp.o: /Users/ugovaretto/projects/s3client/test/utility.cpp
test/CMakeFiles/bucket-test.dir/utility.cpp.o: test/CMakeFiles/bucket-test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/ugovaretto/projects/s3client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object test/CMakeFiles/bucket-test.dir/utility.cpp.o"
	cd /Users/ugovaretto/projects/s3client/build/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/bucket-test.dir/utility.cpp.o -MF CMakeFiles/bucket-test.dir/utility.cpp.o.d -o CMakeFiles/bucket-test.dir/utility.cpp.o -c /Users/ugovaretto/projects/s3client/test/utility.cpp

test/CMakeFiles/bucket-test.dir/utility.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bucket-test.dir/utility.cpp.i"
	cd /Users/ugovaretto/projects/s3client/build/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/ugovaretto/projects/s3client/test/utility.cpp > CMakeFiles/bucket-test.dir/utility.cpp.i

test/CMakeFiles/bucket-test.dir/utility.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bucket-test.dir/utility.cpp.s"
	cd /Users/ugovaretto/projects/s3client/build/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/ugovaretto/projects/s3client/test/utility.cpp -o CMakeFiles/bucket-test.dir/utility.cpp.s

# Object files for target bucket-test
bucket__test_OBJECTS = \
"CMakeFiles/bucket-test.dir/api/bucket-test.cpp.o" \
"CMakeFiles/bucket-test.dir/utility.cpp.o"

# External object files for target bucket-test
bucket__test_EXTERNAL_OBJECTS =

build/debug/test/bucket-test: test/CMakeFiles/bucket-test.dir/api/bucket-test.cpp.o
build/debug/test/bucket-test: test/CMakeFiles/bucket-test.dir/utility.cpp.o
build/debug/test/bucket-test: test/CMakeFiles/bucket-test.dir/build.make
build/debug/test/bucket-test: lib/libs3client.a
build/debug/test/bucket-test: test/CMakeFiles/bucket-test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/ugovaretto/projects/s3client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../build/debug/test/bucket-test"
	cd /Users/ugovaretto/projects/s3client/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bucket-test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/bucket-test.dir/build: build/debug/test/bucket-test
.PHONY : test/CMakeFiles/bucket-test.dir/build

test/CMakeFiles/bucket-test.dir/clean:
	cd /Users/ugovaretto/projects/s3client/build/test && $(CMAKE_COMMAND) -P CMakeFiles/bucket-test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/bucket-test.dir/clean

test/CMakeFiles/bucket-test.dir/depend:
	cd /Users/ugovaretto/projects/s3client/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/ugovaretto/projects/s3client /Users/ugovaretto/projects/s3client/test /Users/ugovaretto/projects/s3client/build /Users/ugovaretto/projects/s3client/build/test /Users/ugovaretto/projects/s3client/build/test/CMakeFiles/bucket-test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/bucket-test.dir/depend

