#*******************************************************************************
# BSD 3-Clause License
# 
# Copyright (c) 2020-2023, Ugo Varetto
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#*******************************************************************************
cmake_minimum_required(VERSION 3.15)
if(${CMAKE_VERSION} VERSION_LESS 3.15)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(s3-client
        VERSION 0.9.8
        DESCRIPTION "Tools to sign and send S3 requests and to perform parallel object upload/download"
        LANGUAGES CXX)

option(APPS "Build Apps" ON)

set(S3LIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/lib/include" CACHE PATH "${PROJECT_SOURCE_DIR}/lib/include")
set(LYRA_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/dep/Lyra/include" CACHE PATH "${PROJECT_SOURCE_DIR}/dep/Lyra/include")
set(TINYXML2_DIR "${PROJECT_SOURCE_DIR}/dep/tinyxml2" CACHE PATH "${PROJECT_SOURCE_DIR}/dep/tinyxml2")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Separate debug-release paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/build/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/build/release)

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    set(LINK_DIRS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} CACHE PATH "")
else (CMAKE_BUILD_TYPE EQUAL "RELEASE")
    set(LINK_DIRS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} CACHE PATH "")
endif (CMAKE_BUILD_TYPE EQUAL "DEBUG")


add_subdirectory(lib)
if(APPS)
add_subdirectory(apps)
endif(APPS)
add_subdirectory(test)
