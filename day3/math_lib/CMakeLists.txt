cmake_minimum_required(VERSION 3.10)
project(MathLibrary)

add_library(my_math STATIC my_math.cpp)

# 让链接此库的目标自动包含当前目录（即能找到 my_math.h）
target_include_directories(my_math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
