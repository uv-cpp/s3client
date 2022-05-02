# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/ubuntu/projects/s3-rest-cpp/CMakeFiles /home/ubuntu/projects/s3-rest-cpp//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/ubuntu/projects/s3-rest-cpp/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named hash_lib

# Build rule for target.
hash_lib: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 hash_lib
.PHONY : hash_lib

# fast build rule for target.
hash_lib/fast:
	$(MAKE) $(MAKESILENT) -f dep/hash/CMakeFiles/hash_lib.dir/build.make dep/hash/CMakeFiles/hash_lib.dir/build
.PHONY : hash_lib/fast

#=============================================================================
# Target rules for targets named s3-download

# Build rule for target.
s3-download: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 s3-download
.PHONY : s3-download

# fast build rule for target.
s3-download/fast:
	$(MAKE) $(MAKESILENT) -f src/CMakeFiles/s3-download.dir/build.make src/CMakeFiles/s3-download.dir/build
.PHONY : s3-download/fast

#=============================================================================
# Target rules for targets named s3-upload

# Build rule for target.
s3-upload: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 s3-upload
.PHONY : s3-upload

# fast build rule for target.
s3-upload/fast:
	$(MAKE) $(MAKESILENT) -f src/CMakeFiles/s3-upload.dir/build.make src/CMakeFiles/s3-upload.dir/build
.PHONY : s3-upload/fast

#=============================================================================
# Target rules for targets named s3-client

# Build rule for target.
s3-client: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 s3-client
.PHONY : s3-client

# fast build rule for target.
s3-client/fast:
	$(MAKE) $(MAKESILENT) -f src/CMakeFiles/s3-client.dir/build.make src/CMakeFiles/s3-client.dir/build
.PHONY : s3-client/fast

#=============================================================================
# Target rules for targets named sign-headers

# Build rule for target.
sign-headers: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 sign-headers
.PHONY : sign-headers

# fast build rule for target.
sign-headers/fast:
	$(MAKE) $(MAKESILENT) -f src/CMakeFiles/sign-headers.dir/build.make src/CMakeFiles/sign-headers.dir/build
.PHONY : sign-headers/fast

#=============================================================================
# Target rules for targets named s3-presign

# Build rule for target.
s3-presign: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 s3-presign
.PHONY : s3-presign

# fast build rule for target.
s3-presign/fast:
	$(MAKE) $(MAKESILENT) -f src/CMakeFiles/s3-presign.dir/build.make src/CMakeFiles/s3-presign.dir/build
.PHONY : s3-presign/fast

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... hash_lib"
	@echo "... s3-client"
	@echo "... s3-download"
	@echo "... s3-presign"
	@echo "... s3-upload"
	@echo "... sign-headers"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

