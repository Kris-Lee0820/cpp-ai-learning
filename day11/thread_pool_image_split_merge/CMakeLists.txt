cmake_minimum_required(VERSION 3.10)
project(ParallelBlockThreadpool)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_executable(parallel_block_threadpool parallel_block_threadpool.cpp)
target_link_libraries(parallel_block_threadpool ${OpenCV_LIBS})
