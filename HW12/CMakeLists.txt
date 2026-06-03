# CMake最低版本号要求
cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
add_compile_options(-w)

# 项目信息
project(FDMJ2026 VERSION 0.1)

# 添加包含路径
include_directories(include/ir)
include_directories(include/instr)
include_directories(include/reg)
include_directories(include/util)
include_directories(vendor/tinyxml2)

# 递归构建
add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(vendor)