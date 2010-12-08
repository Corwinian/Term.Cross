#
#
#

project(TEST_PNG)
cmake_minimum_required(VERSION 2.6)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

add_executable(pngexample pngexample.cpp)
add_executable(pngtest pngtest.cpp)

set_target_properties( pngexample pngtest PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (pngexample ${PNG_LIBRARIES})
target_link_libraries (pngtest ${PNG_LIBRARIES})

#install(TARGETS pngexample pngtest
#        RUNTIME DESTINATION bin
#        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib)
#INSTALL(FILES ${HEADERS} DESTINATION include)
