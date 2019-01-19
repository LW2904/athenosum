cmake_minimum_required (VERSION 3.13)

project(libathenosum VERSION 0.0.1 LANGUAGES CXX)

include(GNUInstallDirs)

# add_library(athenosum SHARED)

target_include_directories(athenosum
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(UNIX AND NOT APPLE)
	find_package(X11 REQUIRED)

	target_include_directories(athenosum PRIVATE ${X11_INCLUDE_DIR})
	target_link_libraries(athenosum PRIVATE ${X11_LIBRARIES} ${X11_Xtst_LIB})
endif()

target_compile_options(athenosum PRIVATE $<$<C_COMPILER_ID:GNU>:-Wall>)

install(TARGETS athenosum
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
