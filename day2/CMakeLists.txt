cmake_minimum_required(VERSION 3.10)
project(Day2Project)

#设置c++标准为c++17
set(CMAKE_cxx_STANDARD 17)
set(CMAKE_cxx_STANDARD_REQUIRED ON)

#添加可执行文件
add_executable(hello_day2 src/main.cpp)
