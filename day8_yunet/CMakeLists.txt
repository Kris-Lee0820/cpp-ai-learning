cmake_minimum_required(VERSION 3.10)
project(VideoProcess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
add_executable(video_process video_demo_yunet.cpp)
target_link_libraries(video_process ${OpenCV_LIBS})
