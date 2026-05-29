cmake_minimum_required(VERSION 3.10)
project(VideoProcess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
if (NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found, please install libopencv-dev")
endif()

add_executable(video_process main.cpp)
target_link_libraries(video_process ${OpenCV_LIBS})

# 可选：设置 release 优化
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(video_process PRIVATE -O3)
endif()
