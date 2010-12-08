#
#
#

project(AUFITCHIP)
cmake_minimum_required(VERSION 2.6)

FILE(GLOB_RECURSE HEADERS include *.hpp)
FILE(GLOB SOURCES src/*.cpp)

message("AUFITCHIP")
message("sources:")
foreach(SRCFILE ${SOURCES})
	message("${SRCFILE}")
endforeach(SRCFILE)
message("include:")
foreach(SRCFILE ${HEADERS})
	message("${SRCFILE}")
endforeach(SRCFILE)

configure_file(${AUFITCHIP_SOURCE_DIR}/etc/aufitchip.cfg.in ${AUFITCHIP_SOURCE_DIR}/etc/aufitchip.cfg)

include_directories (include)
include_directories (${C_LIB_SOURCE_DIR}/include)
include_directories (${NRC_SOURCE_DIR}/include)
include_directories (${TGDB_SOURCE_DIR}/include)
include_directories (${LOG_SOURCE_DIR}/include)
include_directories (${ORBMODEL_SOURCE_DIR}/include)
include_directories (${Y_UTIL_SOURCE_DIR}/include)

add_executable(aufitchip ${SOURCES})

set_target_properties(aufitchip PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (aufitchip c_lib orbmodel y_util tgdb nrc log)

install(TARGETS aufitchip
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

#INSTALL(FILES ${HEADERS} DESTINATION include)

INSTALL(FILES etc/aufitchip.cfg DESTINATION bin)
INSTALL(DIRECTORY chipbase DESTINATION data PATTERN "*.svn" EXCLUDE)
