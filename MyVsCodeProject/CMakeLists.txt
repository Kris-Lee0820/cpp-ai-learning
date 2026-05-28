cmake_minimum_required(VERSION 3.10.0)
project(MyTestProject VERSION 0.1.0 LANGUAGES C CXX)

add_compile_options(/utf-8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MyTestProject main.cpp)

