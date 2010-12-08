#
#
#

project(TESTS)
cmake_minimum_required(VERSION 2.6)

if(BUILD_GENMETA)
add_subdirectory(png)
endif(BUILD_GENMETA)

add_subdirectory(cfg)
