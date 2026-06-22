CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
project(test_rw_lock)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(test_rw_lock test_rw_lock.cpp)