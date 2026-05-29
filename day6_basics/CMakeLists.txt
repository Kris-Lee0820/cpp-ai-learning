cmake_minimum_required(VERSION 3.10)
project(ConfigDemo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(demo main.cpp)

# 使用生成器表达式根据构建类型定义宏和编译选项
# target_compile_definitions()
# 作用：为编译器添加预处理定义（Preprocessor Definitions）。
# 等价于：在编译命令行中插入 -D 或 /D 选项。
# 用途：控制条件编译（如 #ifdef）、传递宏值等。
target_compile_definitions(demo PRIVATE
	$<$<CONFIG:Debug>:DEBUG_MODE>
	$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
)

# target_compile_options
# 作用：直接为编译器传递命令行选项。
# 等价于：任意编译标志（不限于 -D）。
# 用途：指定警告级别（-Wall）、优化选项（-O2）、语言标准（-std=c++17）、搜索路径（-I，但通常用 target_include_directories）等。

target_compile_options(demo PRIVATE
	$<$<CONFIG:Debug>:-g -O0>
	$<$<CONFIG:Release>:-O3>
)

# 可选：打印当前配置
message(STATUS "Building for configuration:  ${CMAK_BUILE_TYPE}")
