# Install script for directory: /home/ugovaretto/projects/uv-cpp/s3client/apps

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/s3-presign" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/s3-presign")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/local/bin/s3-presign"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/bin/s3-presign")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/bin" TYPE EXECUTABLE FILES "/home/ugovaretto/projects/uv-cpp/s3client/build/build/debug/s3-presign")
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/s3-presign" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/s3-presign")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/usr/local/bin/s3-presign"
         OLD_RPATH "/home/ugovaretto/projects/uv-cpp/s3client/build/build/release:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/bin/s3-presign")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/s3-client" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/s3-client")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/local/bin/s3-client"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/bin/s3-client")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/bin" TYPE EXECUTABLE FILES "/home/ugovaretto/projects/uv-cpp/s3client/build/build/debug/s3-client")
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/s3-client" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/s3-client")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/usr/local/bin/s3-client"
         OLD_RPATH "/home/ugovaretto/projects/uv-cpp/s3client/build/build/release:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/bin/s3-client")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/s3-upload" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/s3-upload")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/local/bin/s3-upload"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/bin/s3-upload")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/bin" TYPE EXECUTABLE FILES "/home/ugovaretto/projects/uv-cpp/s3client/build/build/debug/s3-upload")
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/s3-upload" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/s3-upload")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/usr/local/bin/s3-upload"
         OLD_RPATH "/home/ugovaretto/projects/uv-cpp/s3client/build/build/release:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/bin/s3-upload")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/s3-download" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/s3-download")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/local/bin/s3-download"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/bin/s3-download")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/bin" TYPE EXECUTABLE FILES "/home/ugovaretto/projects/uv-cpp/s3client/build/build/debug/s3-download")
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/s3-download" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/s3-download")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/usr/local/bin/s3-download"
         OLD_RPATH "/home/ugovaretto/projects/uv-cpp/s3client/build/build/release:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/bin/s3-download")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/s3-gen-credentials" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/s3-gen-credentials")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/local/bin/s3-gen-credentials"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/bin/s3-gen-credentials")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/bin" TYPE EXECUTABLE FILES "/home/ugovaretto/projects/uv-cpp/s3client/build/build/debug/s3-gen-credentials")
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/s3-gen-credentials" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/s3-gen-credentials")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/bin/s3-gen-credentials")
    endif()
  endif()
endif()

