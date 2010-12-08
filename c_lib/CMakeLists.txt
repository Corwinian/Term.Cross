#
#
#

project(C_LIB)
cmake_minimum_required(VERSION 2.6)

FILE(GLOB_RECURSE HEADERS include *.hpp)
FILE(GLOB SOURCES src/*.cpp)

message("C_LIB")
message("sources:")
foreach(SRCFILE ${SOURCES})
	message("${SRCFILE}")
endforeach(SRCFILE)
message("include:")
foreach(SRCFILE ${HEADERS})
	message("${SRCFILE}")
endforeach(SRCFILE)

include_directories(include)

add_library(c_lib ${SOURCES})

set_target_properties(c_lib PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS c_lib     RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib)
#INSTALL(FILES ${HEADERS} DESTINATION include)
