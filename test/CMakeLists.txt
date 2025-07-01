set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # 动态库 与 可执行文件 放在一起
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # 静态库单独存放

add_executable(tga_image_test 
    ${CMAKE_SOURCE_DIR}/test/unit/tga_image_test.cpp
)
target_include_directories(tga_image_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/test/include
)
target_link_libraries(tga_image_test PRIVATE 
    SR::SoftRasterizer
)

add_test(NAME UnitTest.tga_image_test COMMAND tga_image_test)
set_tests_properties(UnitTest.tga_image_test PROPERTIES 
    LABELS "Unit"
)