cmake_minimum_required(VERSION 3.10)
project(UseInstalled)

#find_package(my_math REQUIRED)   # 但我们没有提供cmake config文件，所以下面手动指定

# 查找库文件
find_library(MY_MATH_LIB my_math PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../day4/install/lib NO_DEFAULT_PATH)
if(NOT MY_MATH_LIB)
    message(FATAL_ERROR "找不到my_math库")
endif()

add_executable(use_installed main.cpp)
target_include_directories(use_installed PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../day4/install/include)
target_link_libraries(use_installed PRIVATE ${MY_MATH_LIB})
