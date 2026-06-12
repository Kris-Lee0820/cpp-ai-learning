cmake_minimum_required(VERSION 3.10)
project(ParallelBatchAsync)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_executable(parallel_batch_async parallel_batch_async.cpp)
target_link_libraries(parallel_batch_async ${OpenCV_LIBS})
