#
#
#

project(NOAAUTIL)
cmake_minimum_required(VERSION 2.6)

FILE(GLOB_RECURSE HEADERS include *.hpp)
FILE(GLOB SOURCES src/*.cpp)
FILE(GLOB RESOURCES data/*.msk)

message("NOAAUTIL")
message("sources:")
foreach(SRCFILE ${SOURCES})
	message("${SRCFILE}")
endforeach(SRCFILE)
message("include:")
foreach(SRCFILE ${HEADERS})
	message("${SRCFILE}")
endforeach(SRCFILE)
message("resources:")
foreach(SRCFILE ${RESOURCES})
	message("${SRCFILE}")
endforeach(SRCFILE)

include_directories(include)
include_directories (${C_LIB_SOURCE_DIR}/include)
include_directories (${HRPT_SOURCE_DIR}/include)
include_directories (${LOG_SOURCE_DIR}/include)
include_directories (${ORBMODEL_SOURCE_DIR}/include)
include_directories (${Y_UTIL_SOURCE_DIR}/include)

#c_lib hrpt log orbmodel y_util

add_executable(h2old src/h2old.cpp)
add_executable(hprj src/hprj.cpp)
add_executable(hunp src/hunp.cpp)

set_target_properties(h2old hprj hunp PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (h2old c_lib hrpt log orbmodel y_util)
target_link_libraries (hprj c_lib hrpt log orbmodel y_util)
target_link_libraries (hunp c_lib hrpt log orbmodel y_util)

install(TARGETS h2old hprj hunp  RUNTIME DESTINATION bin
                                 LIBRARY DESTINATION lib
                                 ARCHIVE DESTINATION lib)
#INSTALL(FILES ${HEADERS} DESTINATION include)
INSTALL(FILES ${RESOURCES} DESTINATION data)
