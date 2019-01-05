cmake_minimum_required (VERSION 3.9)

project(athenosum)
set(CMAKE_BUILD_TYPE Release)

add_library(athenosum SHARED
        src/beatmap.c)

set_target_properties(athenosum PROPERTIES PUBLIC_HEADER
        src/beatmap.h)

target_include_directories(athenosum PRIVATE src)

include(GNUInstallDirs)

install(TARGETS athenosum
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)