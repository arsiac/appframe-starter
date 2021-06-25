cmake_minimum_required(VERSION 3.19)
project(appframe_starter)

set(CMAKE_CXX_STANDARD 14)

add_library(lib_properties
        common/Properties.cpp)

target_include_directories(lib_properties
        PUBLIC
        ${PROJECT_SOURCE_DIR}/common)

### appframe starter
add_executable(appframe_starter  main.cpp afdef.h common/common.h)

target_link_libraries(appframe_starter
        PRIVATE
        lib_properties)