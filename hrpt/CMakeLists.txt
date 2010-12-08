#
#
#

project(HRPT)
cmake_minimum_required(VERSION 2.6)

FILE(GLOB_RECURSE HEADERS include *.hpp)
FILE(GLOB SOURCES src/*.cpp)

message("HRPT")
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

add_library(hrpt ${SOURCES})

set_target_properties(hrpt PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (hrpt c_lib)

install(TARGETS hrpt      RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib)
#INSTALL(FILES ${HEADERS} DESTINATION include)
