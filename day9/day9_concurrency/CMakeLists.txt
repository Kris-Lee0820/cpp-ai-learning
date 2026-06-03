cmake_minimum_required(VERSION 3.10)
project(ConcurrencyDemo)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)
add_executable(blur_parallel main.cpp)
target_link_libraries(blur_parallel ${OpenCV_LIBS} pthread)
