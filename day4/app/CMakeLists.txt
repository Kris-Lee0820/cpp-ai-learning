cmake_minimum_required(VERSION 3.10)
project(MyApp)

add_executable(day3_app main.cpp)

# 关键：链接 my_math 库
target_link_libraries(day3_app PRIVATE my_math) #fmt::fmt --暂时不添加到my_math的后方，因为网络问题无法在git上通过CMake的Fetch Content获取内容的方式来直接下载库文件并使用
