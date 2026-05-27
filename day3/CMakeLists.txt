cmake_minimum_required(VERSION 3.14)
project(ModularProject)

# set c++ standard is 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 引入Fetch Content模块[reference:4]
include(FetchContent)

# 声明我们要获取fmt库[reference:5]
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GTI_TAG 10.1.0     # 使用一个稳定的版本号
)
FetchContent_MakeAvailable(fmt)

# 添加子目录，这里的顺序很中要，app模块依赖于math_lib，所以app应该后添加
add_subdirectory(math_lib)
add_subdirectory(app)
