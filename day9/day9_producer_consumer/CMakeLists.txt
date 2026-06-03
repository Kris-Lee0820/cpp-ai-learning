cmake_minimum_required(VERSION 3.10)
project(ConsumerProducer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 0N)

find_package(OpenCV REQUIRED)
add_executable(producer_consumer producer_consumer.cpp)
target_link_libraries(producer_consumer ${OpenCV_LIBS} pthread)
