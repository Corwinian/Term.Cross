#
#
#

project(ORBMODEL)
cmake_minimum_required(VERSION 2.6)

FILE(GLOB_RECURSE HEADERS include *.hpp)
FILE(GLOB SOURCES src/*.cpp)

message("ORBMODEL")
message("sources:")
foreach(SRCFILE ${SOURCES})
	message("${SRCFILE}")
endforeach(SRCFILE)
message("include:")
foreach(SRCFILE ${HEADERS})
	message("${SRCFILE}")
endforeach(SRCFILE)

include_directories(include)
include_directories (${C_LIB_SOURCE_DIR}/include)
include_directories (${Y_UTIL_SOURCE_DIR}/include)

add_library(orbmodel ${SOURCES})

set_target_properties(orbmodel PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS orbmodel  RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib)
#INSTALL(FILES ${HEADERS} DESTINATION include)
