#
#
#

project(STRFORMAT)
cmake_minimum_required(VERSION 2.6)
message("STRFORMAT")
message("sources:")
message("strformat.cpp")
message("include:")
message("strformat.hpp")
include_directories(.)
include_directories (${C_LIB_SOURCE_DIR}/include)
add_library(strformat strformat.cpp)
set_target_properties(strformat PROPERTIES LINKER_LANGUAGE C)
install(TARGETS strformat RUNTIME DESTINATION bin
 LIBRARY DESTINATION lib
 ARCHIVE DESTINATION lib)
