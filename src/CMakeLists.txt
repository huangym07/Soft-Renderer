set(SoftRasterizer_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/util/tga_image.cpp
)
set(SoftRasterizer_HEADER_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # 动态库 与 可执行文件 放在一起
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # 静态库单独存放

add_library(SoftRasterizer SHARED ${SoftRasterizer_SOURCES})
add_library(SR::SoftRasterizer ALIAS SoftRasterizer)
target_include_directories(SoftRasterizer PUBLIC ${SoftRasterizer_HEADER_DIRS})


add_executable(renderer ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(renderer PRIVATE SR::SoftRasterizer)