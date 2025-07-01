cmake_minimum_required(VERSION 3.28)

project(SoftRenderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "配置 ${CMAKE_BUILD_TYPE} 构建")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g -DDEBUG)
endif()


# 软光栅库
add_subdirectory(src)


# 构建测试
option(BUILD_TESTS "构建测试" ON)
message(STATUS "BUILD_TESTS=${BUILD_TESTS}")
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test EXCLUDE_FROM_ALL)
endif()