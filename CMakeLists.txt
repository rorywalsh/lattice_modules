cmake_minimum_required(VERSION 3.15)
project (LatticeModules)

if(NOT DEFINED BUILD_VERSION)
	set(BUILD_VERSION 0.0.42)
endif()

set(SOURCE src/LatticeProcessorModule.h)

if(MSVC)
    set(PLATFORM "Windows")
else()
    set(PLATFORM "MacOS")
endif()

source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX "Source" FILES ${SOURCE})

add_compile_options ( -fdeclspec -fms-extensions)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories("${CMAKE_SOURCE_DIR}/aurora/include")

add_subdirectory(${CMAKE_SOURCE_DIR}/gain)
add_subdirectory(${CMAKE_SOURCE_DIR}/waveform_viewer)
add_subdirectory(${CMAKE_SOURCE_DIR}/simple_synth)



