# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ubuntu/projects/s3-rest-cpp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/projects/s3-rest-cpp

# Include any dependencies generated for this target.
include dep/hash/CMakeFiles/hash_lib.dir/depend.make

# Include the progress variables for this target.
include dep/hash/CMakeFiles/hash_lib.dir/progress.make

# Include the compile flags for this target's objects.
include dep/hash/CMakeFiles/hash_lib.dir/flags.make

dep/hash/CMakeFiles/hash_lib.dir/crc32.cpp.o: dep/hash/CMakeFiles/hash_lib.dir/flags.make
dep/hash/CMakeFiles/hash_lib.dir/crc32.cpp.o: dep/hash/crc32.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/projects/s3-rest-cpp/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object dep/hash/CMakeFiles/hash_lib.dir/crc32.cpp.o"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/hash_lib.dir/crc32.cpp.o -c /home/ubuntu/projects/s3-rest-cpp/dep/hash/crc32.cpp

dep/hash/CMakeFiles/hash_lib.dir/crc32.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hash_lib.dir/crc32.cpp.i"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/projects/s3-rest-cpp/dep/hash/crc32.cpp > CMakeFiles/hash_lib.dir/crc32.cpp.i

dep/hash/CMakeFiles/hash_lib.dir/crc32.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hash_lib.dir/crc32.cpp.s"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/projects/s3-rest-cpp/dep/hash/crc32.cpp -o CMakeFiles/hash_lib.dir/crc32.cpp.s

dep/hash/CMakeFiles/hash_lib.dir/digest.cpp.o: dep/hash/CMakeFiles/hash_lib.dir/flags.make
dep/hash/CMakeFiles/hash_lib.dir/digest.cpp.o: dep/hash/digest.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/projects/s3-rest-cpp/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object dep/hash/CMakeFiles/hash_lib.dir/digest.cpp.o"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/hash_lib.dir/digest.cpp.o -c /home/ubuntu/projects/s3-rest-cpp/dep/hash/digest.cpp

dep/hash/CMakeFiles/hash_lib.dir/digest.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hash_lib.dir/digest.cpp.i"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/projects/s3-rest-cpp/dep/hash/digest.cpp > CMakeFiles/hash_lib.dir/digest.cpp.i

dep/hash/CMakeFiles/hash_lib.dir/digest.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hash_lib.dir/digest.cpp.s"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/projects/s3-rest-cpp/dep/hash/digest.cpp -o CMakeFiles/hash_lib.dir/digest.cpp.s

dep/hash/CMakeFiles/hash_lib.dir/keccak.cpp.o: dep/hash/CMakeFiles/hash_lib.dir/flags.make
dep/hash/CMakeFiles/hash_lib.dir/keccak.cpp.o: dep/hash/keccak.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/projects/s3-rest-cpp/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object dep/hash/CMakeFiles/hash_lib.dir/keccak.cpp.o"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/hash_lib.dir/keccak.cpp.o -c /home/ubuntu/projects/s3-rest-cpp/dep/hash/keccak.cpp

dep/hash/CMakeFiles/hash_lib.dir/keccak.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hash_lib.dir/keccak.cpp.i"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/projects/s3-rest-cpp/dep/hash/keccak.cpp > CMakeFiles/hash_lib.dir/keccak.cpp.i

dep/hash/CMakeFiles/hash_lib.dir/keccak.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hash_lib.dir/keccak.cpp.s"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/projects/s3-rest-cpp/dep/hash/keccak.cpp -o CMakeFiles/hash_lib.dir/keccak.cpp.s

dep/hash/CMakeFiles/hash_lib.dir/md5.cpp.o: dep/hash/CMakeFiles/hash_lib.dir/flags.make
dep/hash/CMakeFiles/hash_lib.dir/md5.cpp.o: dep/hash/md5.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/projects/s3-rest-cpp/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object dep/hash/CMakeFiles/hash_lib.dir/md5.cpp.o"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/hash_lib.dir/md5.cpp.o -c /home/ubuntu/projects/s3-rest-cpp/dep/hash/md5.cpp

dep/hash/CMakeFiles/hash_lib.dir/md5.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hash_lib.dir/md5.cpp.i"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/projects/s3-rest-cpp/dep/hash/md5.cpp > CMakeFiles/hash_lib.dir/md5.cpp.i

dep/hash/CMakeFiles/hash_lib.dir/md5.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hash_lib.dir/md5.cpp.s"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/projects/s3-rest-cpp/dep/hash/md5.cpp -o CMakeFiles/hash_lib.dir/md5.cpp.s

dep/hash/CMakeFiles/hash_lib.dir/sha1.cpp.o: dep/hash/CMakeFiles/hash_lib.dir/flags.make
dep/hash/CMakeFiles/hash_lib.dir/sha1.cpp.o: dep/hash/sha1.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/projects/s3-rest-cpp/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object dep/hash/CMakeFiles/hash_lib.dir/sha1.cpp.o"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/hash_lib.dir/sha1.cpp.o -c /home/ubuntu/projects/s3-rest-cpp/dep/hash/sha1.cpp

dep/hash/CMakeFiles/hash_lib.dir/sha1.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hash_lib.dir/sha1.cpp.i"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/projects/s3-rest-cpp/dep/hash/sha1.cpp > CMakeFiles/hash_lib.dir/sha1.cpp.i

dep/hash/CMakeFiles/hash_lib.dir/sha1.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hash_lib.dir/sha1.cpp.s"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/projects/s3-rest-cpp/dep/hash/sha1.cpp -o CMakeFiles/hash_lib.dir/sha1.cpp.s

dep/hash/CMakeFiles/hash_lib.dir/sha3.cpp.o: dep/hash/CMakeFiles/hash_lib.dir/flags.make
dep/hash/CMakeFiles/hash_lib.dir/sha3.cpp.o: dep/hash/sha3.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/projects/s3-rest-cpp/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object dep/hash/CMakeFiles/hash_lib.dir/sha3.cpp.o"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/hash_lib.dir/sha3.cpp.o -c /home/ubuntu/projects/s3-rest-cpp/dep/hash/sha3.cpp

dep/hash/CMakeFiles/hash_lib.dir/sha3.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hash_lib.dir/sha3.cpp.i"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/projects/s3-rest-cpp/dep/hash/sha3.cpp > CMakeFiles/hash_lib.dir/sha3.cpp.i

dep/hash/CMakeFiles/hash_lib.dir/sha3.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hash_lib.dir/sha3.cpp.s"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/projects/s3-rest-cpp/dep/hash/sha3.cpp -o CMakeFiles/hash_lib.dir/sha3.cpp.s

dep/hash/CMakeFiles/hash_lib.dir/sha256.cpp.o: dep/hash/CMakeFiles/hash_lib.dir/flags.make
dep/hash/CMakeFiles/hash_lib.dir/sha256.cpp.o: dep/hash/sha256.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/projects/s3-rest-cpp/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object dep/hash/CMakeFiles/hash_lib.dir/sha256.cpp.o"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/hash_lib.dir/sha256.cpp.o -c /home/ubuntu/projects/s3-rest-cpp/dep/hash/sha256.cpp

dep/hash/CMakeFiles/hash_lib.dir/sha256.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hash_lib.dir/sha256.cpp.i"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/projects/s3-rest-cpp/dep/hash/sha256.cpp > CMakeFiles/hash_lib.dir/sha256.cpp.i

dep/hash/CMakeFiles/hash_lib.dir/sha256.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hash_lib.dir/sha256.cpp.s"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/projects/s3-rest-cpp/dep/hash/sha256.cpp -o CMakeFiles/hash_lib.dir/sha256.cpp.s

# Object files for target hash_lib
hash_lib_OBJECTS = \
"CMakeFiles/hash_lib.dir/crc32.cpp.o" \
"CMakeFiles/hash_lib.dir/digest.cpp.o" \
"CMakeFiles/hash_lib.dir/keccak.cpp.o" \
"CMakeFiles/hash_lib.dir/md5.cpp.o" \
"CMakeFiles/hash_lib.dir/sha1.cpp.o" \
"CMakeFiles/hash_lib.dir/sha3.cpp.o" \
"CMakeFiles/hash_lib.dir/sha256.cpp.o"

# External object files for target hash_lib
hash_lib_EXTERNAL_OBJECTS =

dep/hash/libhash_lib.a: dep/hash/CMakeFiles/hash_lib.dir/crc32.cpp.o
dep/hash/libhash_lib.a: dep/hash/CMakeFiles/hash_lib.dir/digest.cpp.o
dep/hash/libhash_lib.a: dep/hash/CMakeFiles/hash_lib.dir/keccak.cpp.o
dep/hash/libhash_lib.a: dep/hash/CMakeFiles/hash_lib.dir/md5.cpp.o
dep/hash/libhash_lib.a: dep/hash/CMakeFiles/hash_lib.dir/sha1.cpp.o
dep/hash/libhash_lib.a: dep/hash/CMakeFiles/hash_lib.dir/sha3.cpp.o
dep/hash/libhash_lib.a: dep/hash/CMakeFiles/hash_lib.dir/sha256.cpp.o
dep/hash/libhash_lib.a: dep/hash/CMakeFiles/hash_lib.dir/build.make
dep/hash/libhash_lib.a: dep/hash/CMakeFiles/hash_lib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ubuntu/projects/s3-rest-cpp/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX static library libhash_lib.a"
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && $(CMAKE_COMMAND) -P CMakeFiles/hash_lib.dir/cmake_clean_target.cmake
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/hash_lib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
dep/hash/CMakeFiles/hash_lib.dir/build: dep/hash/libhash_lib.a

.PHONY : dep/hash/CMakeFiles/hash_lib.dir/build

dep/hash/CMakeFiles/hash_lib.dir/clean:
	cd /home/ubuntu/projects/s3-rest-cpp/dep/hash && $(CMAKE_COMMAND) -P CMakeFiles/hash_lib.dir/cmake_clean.cmake
.PHONY : dep/hash/CMakeFiles/hash_lib.dir/clean

dep/hash/CMakeFiles/hash_lib.dir/depend:
	cd /home/ubuntu/projects/s3-rest-cpp && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/projects/s3-rest-cpp /home/ubuntu/projects/s3-rest-cpp/dep/hash /home/ubuntu/projects/s3-rest-cpp /home/ubuntu/projects/s3-rest-cpp/dep/hash /home/ubuntu/projects/s3-rest-cpp/dep/hash/CMakeFiles/hash_lib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : dep/hash/CMakeFiles/hash_lib.dir/depend

