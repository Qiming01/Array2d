cmake_minimum_required(VERSION 3.14)
project(Array2dIteratorTest)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译选项
if (MSVC)
    add_compile_options(/W4)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# 启用 GMock
set(INSTALL_GTEST OFF)
set(BUILD_GMOCK ON)   # 启用 GMock
set(BUILD_GTEST ON)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # Windows 下的 CRT 设置

FetchContent_MakeAvailable(googletest)

# 查找所有测试源文件
file(GLOB TEST_SOURCES "test_*.cpp")

# 添加测试可执行文件
add_executable(test_array2d ${TEST_SOURCES})

# 包含头文件路径
target_include_directories(test_array2d PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
)

# 链接 GoogleTest 和 GMock
target_link_libraries(test_array2d
        GTest::gtest_main
        GTest::gmock_main
)

# 自动注册测试
include(GoogleTest)
gtest_discover_tests(test_array2d)

# 添加自定义目标来运行测试
add_custom_target(run_tests
        COMMAND test_array2d
        DEPENDS test_array2d
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)