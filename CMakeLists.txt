cmake_minimum_required (VERSION 3.9)

project(libathenosum VERSION 0.0.1 LANGUAGES C)

include(GNUInstallDirs)

add_library(athenosum
		src/beatmap.c
		src/game.c
		src/process.c
		src/window.c)

target_include_directories(athenosum
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(athenosum PRIVATE $<$<C_COMPILER_ID:GNU>:-Wall -D OSU_DEBUG>)

install(TARGETS athenosum
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
