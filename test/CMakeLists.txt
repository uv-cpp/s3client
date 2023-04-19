#*******************************************************************************
# BSD 3-Clause License
# 
# Copyright (c) 2020-2023, Ugo Varetto

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

set(CMAKE_CXX_FLAGS_RELEASE "-flto -Ofast")
set(CMAKE_CXX_FLAGS_DEBUG "-g" )

include_directories(${S3LIB_INCLUDE_DIR}
                    ${HASH_LIB_INCLUDE_DIR}
                    )


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/test")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/test")
link_directories(${LINK_DIRS})

add_executable("bucket-test" api/bucket-test.cpp api/utility.cpp)
add_executable("object-test" api/object-test.cpp api/utility.cpp)
add_executable("fileobject-test" api/fileobject-test.cpp api/utility.cpp)
add_executable("multipart-upload-test" api/multipart-upload-test.cpp api/utility.cpp)
add_executable("multipart-upload-file-test" api/multipart-upload-file-test.cpp api/utility.cpp)
add_executable("abort-multipart-upload-test" api/abort-multipart-upload-test.cpp api/utility.cpp)

target_link_libraries("bucket-test" s3client hash_lib curl) 
target_link_libraries("object-test" s3client hash_lib curl) 
target_link_libraries("fileobject-test" s3client hash_lib curl) 
target_link_libraries("multipart-upload-test" s3client hash_lib curl) 
target_link_libraries("multipart-upload-file-test" s3client hash_lib curl) 
target_link_libraries("abort-multipart-upload-test" s3client hash_lib curl) 

add_executable(sign-test sign-test.cpp)
add_executable(presign-url-test presign-url-test.cpp)
add_executable("xml-parse-test" xml-parse-test.cpp)
target_link_libraries(sign-test s3client hash_lib)
target_link_libraries(presign-url-test s3client hash_lib)
target_link_libraries(xml-parse-test s3client)

#include(GNUInstallDirs)
#install(TARGETS bucket-test object-test multipart-uplaod-test 
#        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/share/sss)
