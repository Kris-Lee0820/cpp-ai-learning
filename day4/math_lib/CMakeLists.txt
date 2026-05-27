cmake_minimum_required(VERSION 3.10)
project(MathLibrary)

add_library(my_math STATIC my_math.cpp)

# 让链接此库的目标自动包含当前目录（即能找到 my_math.h）
target_include_directories(my_math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 安装库文件
install(TARGETS my_math
	ARCHIVE DESTINATION lib   # 静态库 .a 安装到lib
	LIBRARY DESTINATION lib   # 动态库 .so 安装到lib
)

# 安装头文件
install(FILES my_math.h
	DESTINATION include/my_math)
