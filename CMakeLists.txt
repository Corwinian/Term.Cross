#
cmake_minimum_required(VERSION 2.6)

project(TERM_CROSS)

message("${CMAKE_SYSTEM}")
message("${CMAKE_SYSTEM_PROCESSOR}")

# The version number.
set (TERM_CROSS_VERSION_MAJOR 0)
set (TERM_CROSS_VERSION_MINOR 1)

include(CheckIncludeFile)
include(CheckFunctionExists)
include(TestBigEndian)

option(BUILD_SHARED_LIBS "Build with shared libs" ON)
option(BUILD_GENMETA "Build genmeta" ON)

set(TERM_CROSS_INCLUDE ${TERM_CROSS_SOURCE_DIR}/include)

if(UNIX)
	set(DIRD '/')
	set(PATHD ':')
	set(MAX_PATH PATH_MAX)
	set(MAX_FNAME NAME_MAX)
	set(MAX_EXT NAME_MAX)
	set(MAX_DIR NAME_MAX)
#	set(MAX_DRIVE "(3)")
	set(MAX_DRIVE 3)
elseif(WIN32)
	set(DIRD '\\\\')
	set(PATHD ';')
	set(MAX_PATH 260)
	set(MAX_FNAME 256)
	set(MAX_EXT 256)
	set(MAX_DIR 256)
	set(MAX_DRIVE 3)
	set(HAVE_DRIVE ON)
else(UNIX)
	message(FATAL_ERROR "Unknown system!!")
endif(UNIX)

test_big_endian(ISBIGE)
message("big endian = ${ISBIGE}")
message("sizeof(void*) = ${CMAKE_SIZEOF_VOID_P}")

if(ISBIGE)
message("big endian = ${ISBIGE}")
endif(ISBIGE)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
message("4 = ${CMAKE_SIZEOF_VOID_P}")
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

if( NOT ISBIGE AND ( CMAKE_SIZEOF_VOID_P EQUAL 4))
	set(__PLATFORM__ 0)
elseif( NOT ISBIGE AND ( CMAKE_SIZEOF_VOID_P EQUAL 8))
	set(__PLATFORM__ 1)
elseif(ISBIGE AND ( CMAKE_SIZEOF_VOID_P EQUAL 4))
	set(__PLATFORM__ 2)
elseif( ISBIGE AND ( CMAKE_SIZEOF_VOID_P EQUAL 8))
	set(__PLATFORM__ 3)
else ( NOT ISBIGE AND ( CMAKE_SIZEOF_VOID_P EQUAL 4))
	message(FATAL_ERROR "Unknown system!!")
endif( NOT ISBIGE AND ( CMAKE_SIZEOF_VOID_P EQUAL 4))

message("__PLATFORM__ = ${__PLATFORM__}")

check_include_file(limits.h HAVE_LIMITS_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_function_exists(strcasecmp STRCASECMP)
check_function_exists(stricmp STRICMP)
#if(BUILD_GENMETA)
#	find_package(PNG REQUIRED)
#	include_directories(${PNG_INCLUDE_DIR})
#endif(BUILD_GENMETA)

configure_file(${TERM_CROSS_INCLUDE}/config.h.in ${TERM_CROSS_INCLUDE}/tc_config.h)

include_directories(${TERM_CROSS_INCLUDE})

add_subdirectory(strformat)
add_subdirectory(c_lib)
add_subdirectory(log)
add_subdirectory(nrc)
add_subdirectory(hrpt)
add_subdirectory(tgdb)
add_subdirectory(y_util)
add_subdirectory(orbmodel)
add_subdirectory(b0)
add_subdirectory(noaautil)
add_subdirectory(sd_util)
add_subdirectory(aufitchip)
add_subdirectory(a02lev0)
add_subdirectory(etc)

add_subdirectory(tests)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)

set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION "trunk")
#set (CPACK_PACKAGE_VERSION_MAJOR "${TERM_CROSS_VERSION_MAJOR}")
#set (CPACK_PACKAGE_VERSION_MINOR "${TERM_CROSS_VERSION_MINOR}")
#set (CPACK_PACKAGE_VERSION_PATCH "0")

include (CPack)
