#
#
#

project(B0)
cmake_minimum_required(VERSION 2.6)

FILE(GLOB_RECURSE HEADERS include *.hpp)
FILE(GLOB SOURCES src/*.cpp)

message("B0")
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
include_directories (${LOG_SOURCE_DIR}/include)
include_directories (${Y_UTIL_SOURCE_DIR}/include)
include_directories (${HRPT_SOURCE_DIR}/include)
include_directories (${ORBMODEL_SOURCE_DIR}/include)

#add_executable(b0 ${SOURCES})
add_executable(b0 src/b0.cpp)
set_target_properties(b0 PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (b0 c_lib y_util)

add_executable(b0info src/b0info.cpp)
set_target_properties(b0 PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (b0info c_lib y_util)

set(B0_TARGETS b0 b0info)

if(BUILD_GENMETA)
	find_package(PNG REQUIRED)
	include_directories(${PNG_INCLUDE_DIR})
	add_executable(genmeta src/genmeta.cpp)
	set_target_properties(genmeta PROPERTIES LINKER_LANGUAGE CXX)
	target_link_libraries (genmeta c_lib log hrpt orbmodel y_util ${PNG_LIBRARIES})
	set(B0_TARGETS ${B0_TARGETS} genmeta)
endif(BUILD_GENMETA)



install(TARGETS b0 b0info ${B0_TARGETS}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
#INSTALL(FILES ${HEADERS} DESTINATION include)
