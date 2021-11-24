cmake_minimum_required(VERSION 3.15)
project (LatticeExternalPluginEffect)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
src/LatticeExternalProcessor.cpp
src/LatticeExternalProcessor.h)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source" FILES ${SOURCES})

add_compile_options ( -fdeclspec -fms-extensions)
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

