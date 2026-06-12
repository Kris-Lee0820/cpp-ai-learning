cmake_minimum_required(VERSION 3.10)
project(Parallel_TBB)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_executable(parallel_tbb intel_tbb.cpp)
target_link_libraries(parallel_tbb ${OpenCV_LIBS} tbb)
