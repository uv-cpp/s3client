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
find_package(Threads)

set(S3_API_SRCS src/api/s3-api.cpp src/api/multipart_upload.cpp
    src/api/bucket.cpp src/api/object.cpp src/api/error.cpp)
set(S3_API_SRCS ${S3_API_SRCS} src/api/xml_parser.cpp ${TINYXML2_DIR}/tinyxml2.cpp)

set(S3_CLIENT_LIB_SRCS src/url_utility.cpp src/aws_sign.cpp 
    src/webclient.cpp src/utility.cpp src/s3-client.cpp src/response_parser.cpp
    src/download.cpp  src/upload-api.cpp
    ${S3_API_SRCS})

set(CMAKE_CXX_FLAGS_RELEASE "-flto -Ofast" ${CMAKE_CXX_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_DEBUG "" ${CMAKE_CXX_FLAGS_DEBUG})

include_directories(include)
include_directories(${HASHLIB_INCLUDE_DIR})
include_directories(${TINYXML2_DIR})

add_library(s3client ${S3_CLIENT_LIB_SRCS})
target_link_libraries(s3client curl hash_lib)

include(GNUInstallDirs)
install(TARGETS s3client
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(DIRECTORY ./include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/s3client
        FILES_MATCHING PATTERN *.h)
