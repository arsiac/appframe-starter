cmake_minimum_required(VERSION 3.19)
project(appframe_starter)

set(CMAKE_CXX_STANDARD 14)

add_library(lib_md5
        common/md5.h
        common/md5.cpp)

add_library(lib_properties
        common/Properties.h
        common/Properties.cpp)


### appframe starter
add_executable(appframe_starter afdef.h common.h main.cpp)

target_include_directories(appframe_starter
        PRIVATE
        ${PROJECT_SOURCE_DIR}/common)

target_link_libraries(appframe_starter
        PRIVATE
        lib_md5
        lib_properties)