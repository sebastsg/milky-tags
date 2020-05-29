cmake_minimum_required(VERSION 3.8)

project(milky-tags)

set(ROOT_DIR "${PROJECT_SOURCE_DIR}/../")
set(CMAKE_CXX_STANDARD 17)

include_directories(
	${PROJECT_SOURCE_DIR}/../../nfwk/include
	${PROJECT_SOURCE_DIR}/../../nfwk/thirdparty/include
)

if(CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES Debug Release)
	set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Reset configurations" FORCE)
endif()

file(GLOB_RECURSE SOURCE_CPP_FILES ${PROJECT_SOURCE_DIR}/../source/*.cpp)
file(GLOB_RECURSE HEADER_HPP_FILES ${PROJECT_SOURCE_DIR}/../source/*.hpp)

source_group(Resources ${PROJECT_SOURCE_DIR}/milky-tags.rc)

add_executable(milky-tags WIN32 ${SOURCE_CPP_FILES} ${HEADER_HPP_FILES} ${PROJECT_SOURCE_DIR}/milky-tags.rc)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT milky-tags)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /MP")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /MP")

if(${WIN32})
	set(DEBUG_LINK_LIBRARIES
		debug ${ROOT_DIR}/../nfwk/thirdparty/lib/debug/freetype.lib
		debug ${ROOT_DIR}/../nfwk/thirdparty/lib/debug/glew32sd.lib
		debug ${ROOT_DIR}/../nfwk/thirdparty/lib/debug/libogg.lib
		debug ${ROOT_DIR}/../nfwk/thirdparty/lib/debug/libpng16.lib
		debug ${ROOT_DIR}/../nfwk/thirdparty/lib/debug/libvorbis.lib
		debug ${ROOT_DIR}/../nfwk/thirdparty/lib/debug/libvorbisfile.lib
		debug ${ROOT_DIR}/../nfwk/thirdparty/lib/debug/zlib.lib
		debug opengl32.lib
		debug glu32.lib
		debug ws2_32.lib
		debug ${ROOT_DIR}/../nfwk/lib/debug/nfwk.lib
	)
	set(RELEASE_LINK_LIBRARIES
		optimized ${ROOT_DIR}/../nfwk/thirdparty/lib/release/freetype.lib
		optimized ${ROOT_DIR}/../nfwk/thirdparty/lib/release/glew32s.lib
		optimized ${ROOT_DIR}/../nfwk/thirdparty/lib/release/libogg.lib
		optimized ${ROOT_DIR}/../nfwk/thirdparty/lib/release/libpng16.lib
		optimized ${ROOT_DIR}/../nfwk/thirdparty/lib/release/libvorbis.lib
		optimized ${ROOT_DIR}/../nfwk/thirdparty/lib/release/libvorbisfile.lib
		optimized ${ROOT_DIR}/../nfwk/thirdparty/lib/release/zlib.lib
		optimized opengl32.lib
		optimized glu32.lib
		optimized ws2_32.lib
		optimized ${ROOT_DIR}/../nfwk/lib/release/nfwk.lib
	)
	set(ALL_LINK_LIBRARIES ${DEBUG_LINK_LIBRARIES} ${RELEASE_LINK_LIBRARIES})
	target_link_libraries(milky-tags ${ALL_LINK_LIBRARIES})
endif()
