cmake_minimum_required(VERSION 3.10)
project(TBB_Block_Blur)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_executable(tbb_block_blur tbb_block_blur.cpp)
target_link_libraries(tbb_block_blur ${OpenCV_LIBS} tbb)
