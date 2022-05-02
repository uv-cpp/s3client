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
# Executables

find_package(Threads)

set(PRESIGN_SRCS presign_url.cpp aws_sign.cpp url_utility.cpp utility.cpp)
set(SIGN_HEADER_SRCS sign_header.cpp aws_sign.cpp url_utility.cpp utility.cpp)
set(PAR_UPLOAD_SRCS "parallel_upload.cpp" url_utility.cpp aws_sign.cpp
    webclient.cpp response_parser.cpp utility.cpp)
set(PAR_DLOAD_SRCS "parallel_download.cpp" url_utility.cpp aws_sign.cpp 
    webclient.cpp response_parser.cpp utility.cpp)
set(S3_CLIENT_SRCS "s3-client.cpp" url_utility.cpp aws_sign.cpp 
    webclient.cpp utility.cpp)

set(CMAKE_CXX_FLAGS "-std=c++17 -flto -Ofast" ${CMAKE_CXX_FLAGS})

add_executable("s3-presign" ${PRESIGN_SRCS})
add_executable("sign-headers" ${SIGN_HEADER_SRCS})
add_executable("s3-client" ${S3_CLIENT_SRCS})
add_executable("s3-upload" ${PAR_UPLOAD_SRCS})
add_executable("s3-download" ${PAR_DLOAD_SRCS})

add_dependencies("s3-presign" ${DEPENDENCIES})
target_link_libraries("s3-presign" ${LIBRARIES})
target_link_libraries("s3-presign" -static-libgcc -static-libstdc++)

add_dependencies("sign-headers" ${DEPENDENCIES})
target_link_libraries("sign-headers" ${LIBRARIES})

add_dependencies("s3-client" ${DEPENDENCIES})
target_link_libraries("s3-client" ${LIBRARIES})
target_link_libraries("s3-client" curl)
target_link_libraries("s3-client" -static-libgcc -static-libstdc++)

add_dependencies("s3-upload" ${DEPENDENCIES})
target_link_libraries("s3-upload" ${LIBRARIES})
target_link_libraries("s3-upload" curl)
target_link_libraries("s3-upload" ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries("s3-upload" -static-libgcc -static-libstdc++)

add_dependencies("s3-download" ${DEPENDENCIES})
target_link_libraries("s3-download" ${LIBRARIES})
target_link_libraries("s3-download" curl)
target_link_libraries("s3-download" ${CMAKE_THREAD_LIBS_INIT})

add_compile_definitions(IGNORE_SIGPIPE)
#add_compile_definitions(TIME_UPLOAD)