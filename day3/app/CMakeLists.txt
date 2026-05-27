cmake_minimum_required(VERSION 3.10)
project(MyApp)

add_executable(day3_app main.cpp)

# 关键：链接 my_math 库
target_link_libraries(day3_app PRIVATE my_math fmt::fmt)
