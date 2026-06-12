cmake_minimum_required(VERSION 3.10)
project(Priority_Threadpool)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_executable(priority_threadpool test_priority_pool.cpp)
target_link_libraries(priority_threadpool ${OpenCV_LIBS})
