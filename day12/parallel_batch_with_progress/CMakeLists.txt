CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
project(Parallel_Batch_With_Progress)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_executable(parallel_batch_with_progress parallel_batch_with_progress.cpp)
target_link_libraries(parallel_batch_with_progress ${OpenCV_LIBS})