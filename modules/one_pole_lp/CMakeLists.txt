cmake_minimum_required(VERSION 3.15)
project (OnePole)

include_directories("../include/")
include_directories("${CMAKE_SOURCE_DIR}/aurora/include")

set(SOURCE OnePoleProcessor.cpp OnePoleProcessor.h)

if(MSVC)
    set(PLATFORM "Windows")
else()
    set(PLATFORM "MacOS")
endif()

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source" FILES ${SOURCE})

add_compile_options ( -fdeclspec -fms-extensions)

# Create plugin module
add_library(${PROJECT_NAME} SHARED ${SOURCE})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)


