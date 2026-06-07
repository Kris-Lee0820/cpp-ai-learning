cmake_minimum_required(VERSION 3.10)
project(ImageProcessorDemo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(test_processor test_processor_float.cpp)
target_link_libraries(test_processor ${OpenCV_LIBS})
