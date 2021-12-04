cmake_minimum_required(VERSION 3.15)
project (LatticeExternal)


set(SOURCES src/LatticeExternalProcessor.cpp src/LatticeExternalProcessor.h)
set(SOURCES_STANDALONE src/LatticeExternalTestApp.cpp)

if(MSVC)
    set(PLATFORM "Windows")
else()
    set(PLATFORM "MacOS")
endif()

set(STANDALONE "LatticeTestApp")
source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX "Source" FILES ${SOURCES} ${SOURCES_STANDALONE})

add_compile_options ( -fdeclspec -fms-extensions)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Create standalone test application
add_executable(${STANDALONE} ${SOURCES_STANDALONE})
target_compile_features(${STANDALONE} PRIVATE cxx_std_17)
target_compile_options(${STANDALONE} PRIVATE -D${PLATFORM})
include_directories("/Volumes/SSD_Drive/Mac/libsndfile/build/include")
set(LIB_SNDFILE "/Volumes/SSD_Drive/Mac/libsndfile/build/libsndfile.a")
target_link_libraries(${STANDALONE} ${LIB_SNDFILE})

# Create plugin module
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

add_subdirectory(${CMAKE_SOURCE_DIR}/delay)


