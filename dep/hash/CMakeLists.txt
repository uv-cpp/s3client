# encoding library
add_library(hash_lib 
    crc32.cpp
    digest.cpp
    keccak.cpp
    md5.cpp
    sha1.cpp
    sha3.cpp
    sha256.cpp)
install(TARGETS hash_lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
