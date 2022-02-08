cmake_minimum_required(VERSION 3.15)
project (LatticeNodeCLI)

if(NOT DEFINED BUILD_VERSION)
	set(BUILD_VERSION 0.0.37)
endif()

set(SOURCES src/LatticeProcessorModule.h)
set(SOURCES_STANDALONE src/LatticeExternalTestApp.cpp src/LatticeProcessorModule.h)

if(MSVC)
    set(PLATFORM "Windows")
else()
    set(PLATFORM "MacOS")
endif()

set(STANDALONE "LatticeExternals")
source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX "Source" FILES ${SOURCES} ${SOURCES_STANDALONE})

add_compile_options ( -fdeclspec -fms-extensions)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Create standalone test application
# add_executable(${PROJECT_NAME} ${SOURCES_STANDALONE})
# target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
# target_compile_options(${PROJECT_NAME} PRIVATE -D${PLATFORM})
# include_directories("/Volumes/SSD_Drive/Mac/libsndfile/build/include")
# set(LIB_SNDFILE "/Volumes/SSD_Drive/Mac/libsndfile/build/libsndfile.a")
# target_link_libraries(${PROJECT_NAME} ${LIB_SNDFILE})

include_directories("${CMAKE_SOURCE_DIR}/aurora/include")

add_subdirectory(${CMAKE_SOURCE_DIR}/gain)
add_subdirectory(${CMAKE_SOURCE_DIR}/gain_debug)
add_subdirectory(${CMAKE_SOURCE_DIR}/waveform_viewer)
add_subdirectory(${CMAKE_SOURCE_DIR}/simple_synth)



