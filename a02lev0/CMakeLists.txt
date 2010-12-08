#
#
#

project(A02LEV0)
cmake_minimum_required(VERSION 2.6)

FILE(GLOB_RECURSE HEADERS include *.h)
FILE(GLOB SOURCES src/*.cpp)

message("A02LEV0")
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

#add_executable(a02lev0 ${SOURCES})
add_executable(a02lev0 src/a02lev0.cpp src/path.cpp src/unp810.cpp)
add_executable(check_a0 src/check_a0.cpp)

set_target_properties(a02lev0 check_a0 PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS a02lev0 check_a0
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

#INSTALL(FILES ${HEADERS} DESTINATION include)
