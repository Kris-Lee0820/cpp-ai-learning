cmake_minimum_required(VERSION 3.10)
project(Async_Video_Pipeline)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_executable(async_video_pipeline async_video_pipeline.cpp)
target_link_libraries(async_video_pipeline ${OpenCV_LIBS})
