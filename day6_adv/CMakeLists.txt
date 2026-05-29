cmake_minimum_required(VERSION 3.10)
project(VideoDemo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(video_demo main.cpp)
target_link_libraries(video_demo ${OpenCV_LIBS})

# 可选：自动复制测试视频到构建目录（如果有）
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test.avi)
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test.avi DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
