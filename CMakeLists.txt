cmake_minimum_required(VERSION 3.14)
project(Array2D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含头文件路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# 启用测试
enable_testing()
add_subdirectory(tests)

add_executable(example example.cpp)