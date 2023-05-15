# Install script for directory: /Users/ugovaretto/projects/s3client/apps

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/ugovaretto/tmp/s3client")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/Users/ugovaretto/tmp/s3client/bin/s3-presign")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/Users/ugovaretto/tmp/s3client/bin" TYPE EXECUTABLE FILES "/Users/ugovaretto/projects/s3client/build/build/debug/s3-presign")
  if(EXISTS "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-presign" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-presign")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/ugovaretto/projects/s3client/build/build/release"
      "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-presign")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -u -r "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-presign")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/Users/ugovaretto/tmp/s3client/bin/s3-client")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/Users/ugovaretto/tmp/s3client/bin" TYPE EXECUTABLE FILES "/Users/ugovaretto/projects/s3client/build/build/debug/s3-client")
  if(EXISTS "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-client" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-client")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/ugovaretto/projects/s3client/build/build/release"
      "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-client")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -u -r "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-client")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/Users/ugovaretto/tmp/s3client/bin/s3-upload")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/Users/ugovaretto/tmp/s3client/bin" TYPE EXECUTABLE FILES "/Users/ugovaretto/projects/s3client/build/build/debug/s3-upload")
  if(EXISTS "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-upload" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-upload")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/ugovaretto/projects/s3client/build/build/release"
      "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-upload")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -u -r "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-upload")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/Users/ugovaretto/tmp/s3client/bin/s3-download")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/Users/ugovaretto/tmp/s3client/bin" TYPE EXECUTABLE FILES "/Users/ugovaretto/projects/s3client/build/build/debug/s3-download")
  if(EXISTS "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-download" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-download")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/ugovaretto/projects/s3client/build/build/release"
      "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-download")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -u -r "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-download")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/Users/ugovaretto/tmp/s3client/bin/s3-gen-credentials")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/Users/ugovaretto/tmp/s3client/bin" TYPE EXECUTABLE FILES "/Users/ugovaretto/projects/s3client/build/build/debug/s3-gen-credentials")
  if(EXISTS "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-gen-credentials" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-gen-credentials")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -u -r "$ENV{DESTDIR}/Users/ugovaretto/tmp/s3client/bin/s3-gen-credentials")
    endif()
  endif()
endif()

