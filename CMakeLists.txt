cmake_minimum_required(VERSION 3.28)

project(SoftRenderer)


set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "配置 ${CMAKE_BUILD_TYPE} 构建")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG -Wno-narrowing)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g)
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # 动态库 与 可执行文件 放在一起
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # 静态库单独存放


set(RENDERER_SOURCES
    ${CMAKE_SOURCE_DIR}/src/util/tga_image.cpp
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)
set(RENDERER_INCLUDE
    ${CMAKE_SOURCE_DIR}/include
)

add_executable(renderer ${RENDERER_SOURCES})
target_include_directories(renderer PRIVATE ${RENDERER_INCLUDE})