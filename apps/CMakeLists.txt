#*******************************************************************************
# BSD 3-Clause License
# 
# Copyright (c) 2020-2022, Ugo Varetto
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
find_package(Threads)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(CMAKE_CXX_FLAGS "-std=c++17 -flto -Ofast" ${CMAKE_CXX_FLAGS})

include_directories(${S3LIB_INCLUDE_DIR}
                    ${HASH_LIB_INCLUDE_DIR}
                    ${LYRA_INCLUDE_DIR})
link_directories(${LINK_DIRS})

add_executable("s3-presign" presign_url.cpp)
add_executable("s3-client" s3-client.cpp)
add_executable("s3-upload" parallel_upload.cpp)
add_executable("s3-download" parallel_download.cpp)

target_link_libraries("s3-presign" s3client hash_lib)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries("s3-presign" -static-libgcc -static-libstdc++)
endif()

target_link_libraries("s3-client" s3client hash_lib) 
target_link_libraries("s3-client" curl)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries("s3-client" -static-libgcc -static-libstdc++)
endif()

target_link_libraries("s3-upload" s3client hash_lib)
target_link_libraries("s3-upload" curl)
target_link_libraries("s3-upload" ${CMAKE_THREAD_LIBS_INIT} )
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries("s3-upload" -static-libgcc -static-libstdc++)
endif()

target_link_libraries("s3-download" s3client hash_lib)
target_link_libraries("s3-download" curl)
target_link_libraries("s3-download" ${CMAKE_THREAD_LIBS_INIT})
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries("s3-download" -static-libgcc -static-libstdc++)
endif()

include(GNUInstallDirs)
install(TARGETS s3-presign s3-client s3-upload s3-download
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
