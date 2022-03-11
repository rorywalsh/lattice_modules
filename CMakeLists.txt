cmake_minimum_required(VERSION 3.15)
project (LatticeModules)

if(NOT DEFINED BUILD_VERSION)
	set(BUILD_VERSION 0.0.54)
endif()

# set(SOURCE include/LatticeProcessorModule.h)

if(MSVC)
    set(PLATFORM "Windows")
else()
    set(PLATFORM "MacOS")
endif()

source_group(TREE ${CMAKE_SOURCE_DIR}/include PREFIX "Source" FILES ${SOURCE})

add_compile_options ( -fdeclspec -fms-extensions)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories("${CMAKE_SOURCE_DIR}/include/")
include_directories("${CMAKE_SOURCE_DIR}/aurora/include")

add_subdirectory(${CMAKE_SOURCE_DIR}/bl_synth)
add_subdirectory(${CMAKE_SOURCE_DIR}/chorus)
add_subdirectory(${CMAKE_SOURCE_DIR}/delay)
add_subdirectory(${CMAKE_SOURCE_DIR}/drive_synth)
add_subdirectory(${CMAKE_SOURCE_DIR}/flanger)
add_subdirectory(${CMAKE_SOURCE_DIR}/four_pole_lp)
add_subdirectory(${CMAKE_SOURCE_DIR}/gain)
add_subdirectory(${CMAKE_SOURCE_DIR}/ks_synth)
add_subdirectory(${CMAKE_SOURCE_DIR}/midi_maker)
add_subdirectory(${CMAKE_SOURCE_DIR}/one_pole_lp)
add_subdirectory(${CMAKE_SOURCE_DIR}/param_eq)
add_subdirectory(${CMAKE_SOURCE_DIR}/simple_synth)
add_subdirectory(${CMAKE_SOURCE_DIR}/two_pole_svf)
add_subdirectory(${CMAKE_SOURCE_DIR}/waveform_viewer)




