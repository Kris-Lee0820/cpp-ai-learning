cmake_minimum_required(VERSION 3.10)
project(VideoProcessor)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_executable(video_processor main.cpp)
target_link_libraries(video_processor ${OpenCV_LIBS})
