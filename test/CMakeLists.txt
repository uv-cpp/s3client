#*******************************************************************************
# BSD 3-Clause License
# 
# Copyright (c) 2020-2022, Ugo Varetto

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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(CMAKE_CXX_FLAGS "-std=c++17 -flto -Ofast" ${CMAKE_CXX_FLAGS})

include_directories(${S3LIB_INCLUDE_DIR}
                    ${HASH_LIB_INCLUDE_DIR}
                    ${LYRA_INCLUDE_DIR})
link_directories(${LINK_DIRS})

add_executable("bucket-test" api/bucket-test.cpp)
add_executable("object-test" api/object-test.cpp)
add_executable("multipart-uplaod-test" api/multipart-upload-test.cpp)

target_link_libraries("bucket-test" s3client hash_lib curl) 
target_link_libraries("object-test" s3client hash_lib curl) 
target_link_libraries("multipart-uplaod-test" s3client hash_lib curl) 

#include(GNUInstallDirs)
#install(TARGETS bucket-test object-test multipart-uplaod-test 
#        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/share/sss)
