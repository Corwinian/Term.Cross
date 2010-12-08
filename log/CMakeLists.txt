#
#
#

project(LOG)
cmake_minimum_required(VERSION 2.6)

FILE(GLOB_RECURSE HEADERS include *.hpp)
FILE(GLOB SOURCES src/*.cpp)

message("LOG")
message("sources:")
foreach(SRCFILE ${SOURCES})
	message("${SRCFILE}")
endforeach(SRCFILE)
message("include:")
foreach(SRCFILE ${HEADERS})
	message("${SRCFILE}")
endforeach(SRCFILE)

include_directories(include)

add_library(log ${SOURCES})

set_target_properties(log PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS log       RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib)
#INSTALL(FILES ${HEADERS} DESTINATION include)
