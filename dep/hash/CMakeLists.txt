# encoding library
add_library(hash_lib STATIC
    crc32.cpp
    digest.cpp
    keccak.cpp
    md5.cpp
    sha1.cpp
    sha3.cpp
    sha256.cpp)
    
# Add library to dependencies and library list in parent scope
set(DEPENDENCIES ${DEPENDENCIES} hash_lib PARENT_SCOPE)
set(LIBRARIES ${LIBRARIES} hash_lib PARENT_SCOPE)