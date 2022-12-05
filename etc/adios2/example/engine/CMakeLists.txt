#------------------------------------------------------------------------------#
# Distributed under the OSI-approved Apache License, Version 2.0.  See
# accompanying file Copyright.txt for details.
#------------------------------------------------------------------------------#
set(CMAKE_CXX_STANDARD 17)
include_directories(~/.local/include)
link_directories(~/.local/lib)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
add_library(PluginEngineWrite SHARED
  ExampleWritePlugin.cpp
)
target_link_libraries(PluginEngineWrite adios2_cxx11 adios2_core)

add_library(PluginEngineRead SHARED
  ExampleReadPlugin.cpp
)
target_link_libraries(PluginEngineRead adios2_cxx11 adios2_core)

add_executable(examplePluginEngine_write
  examplePluginEngine_write.cpp
)
target_link_libraries(examplePluginEngine_write adios2_cxx11)

add_executable(examplePluginEngine_read
  examplePluginEngine_read.cpp
)
target_link_libraries(examplePluginEngine_read adios2_cxx11)
