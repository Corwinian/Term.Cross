#
#
#

project( TEST_CFG )
cmake_minimum_required( VERSION 2.6 )

include_directories( ${C_LIB_SOURCE_DIR}/include)

add_executable( cfgtest cfgtest.cpp )

set_target_properties( cfgtest PROPERTIES LINKER_LANGUAGE CXX )

target_link_libraries( cfgtest c_lib )

#install(TARGETS pngexample pngtest
#        RUNTIME DESTINATION bin
#        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib)
#INSTALL(FILES ${HEADERS} DESTINATION include)
