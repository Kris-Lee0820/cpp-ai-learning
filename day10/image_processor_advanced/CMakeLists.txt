cmake_minimum_required(VERSION 3.10)
project(AdvancedImageProcessor)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)

add_executable(test_advanced test_advanced_advanced.cpp)
target_link_libraries(test_advanced ${OpenCV_LIBS})
