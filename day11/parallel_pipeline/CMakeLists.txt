cmake_minimum_required(VERSION 3.10)
project(Parallel_pipeline)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_executable(parallel_pipeline parallel_pipeline.cpp)
target_link_libraries(parallel_pipeline ${OpenCV_LIBS} tbb)
