#
#
#

project(SD_UTIL)
cmake_minimum_required(VERSION 2.6)

FILE(GLOB_RECURSE HEADERS include *.hpp)
FILE(GLOB SOURCES src/*.cpp)
FILE(GLOB RESOURCES data/*.d*)

message("SD_UTIL")
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

include_directories (include)
include_directories (${C_LIB_SOURCE_DIR}/include)
include_directories (${LOG_SOURCE_DIR}/include)
include_directories (${ORBMODEL_SOURCE_DIR}/include)
include_directories (${Y_UTIL_SOURCE_DIR}/include)

add_executable(calibr src/astronom.cpp src/calibr.cpp src/xml.cpp)
add_executable(fcalibr src/fcalibr.cpp)
add_executable(filtr src/astronom.cpp src/filtr.cpp)
add_executable(filtr2 src/astronom.cpp src/filtr2.cpp)
add_executable(multich src/astronom.cpp src/multich.cpp)

set_target_properties(calibr fcalibr filtr filtr2 multich PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (calibr c_lib log orbmodel y_util)
target_link_libraries (filtr c_lib log orbmodel y_util)
target_link_libraries (filtr2 c_lib log orbmodel y_util)
target_link_libraries (multich c_lib log orbmodel y_util)

install(TARGETS calibr fcalibr filtr filtr2 multich
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

#INSTALL(FILES ${HEADERS} DESTINATION include)
INSTALL(FILES ${RESOURCES} DESTINATION data)
