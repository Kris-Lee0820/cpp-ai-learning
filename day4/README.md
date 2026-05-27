# Day 4 (2026-05-29) 学习笔记

## 📌 今日学习内容
1. **解决网络依赖**：注释掉 `FetchContent` 部分，避免因网络问题导致构建失败。
2. **为项目添加单元测试**：使用 CMake 内置的 `enable_testing()` + `add_test()`。
3. **实现安装规则**：使用 `install()` 命令将库和头文件安装到指定目录。
4. **使用已安装的库**：创建外部项目，通过绝对路径或 `find_library` 链接安装后的库。

---

## 🧪 一、添加单元测试

### 1. 创建测试文件 `tests/test_math.cpp`
```cpp
#include <iostream>
#include "my_math.h"

int main() {
    int result = add(2, 3);
    if (result == 5) {
        std::cout << "test_add: PASSED" << std::endl;
        return 0;
    } else {
        std::cout << "test_add: FAILED (got " << result << ")" << std::endl;
        return 1;
    }
}
2. 修改顶层 CMakeLists.txt，启用测试
cmake
# 在 add_subdirectory(app) 之后添加
enable_testing()

add_executable(test_math tests/test_math.cpp)
target_link_libraries(test_math PRIVATE my_math)
target_include_directories(test_math PRIVATE ${CMAKE_SOURCE_DIR}/math_lib)

add_test(NAME MathTest COMMAND test_math)
3. 构建并运行测试
bash
cd ~/cpp-ai-learning/day4
rm -rf build && mkdir build && cd build
cmake ..
make
ctest
预期输出：

text
100% tests passed, 0 tests failed out of 1
📦 二、安装库和头文件
1. 在 math_lib/CMakeLists.txt 末尾添加安装规则
cmake
install(TARGETS my_math
        ARCHIVE DESTINATION lib   # 静态库 .a
        LIBRARY DESTINATION lib   # 动态库 .so
        )
install(FILES my_math.h
        DESTINATION include/my_math)
2. 自定义安装前缀（避免需要 sudo）
bash
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install
make
make install
3. 验证安装结果
bash
ls ../install/lib          # 应看到 libmy_math.a
ls ../install/include/my_math/   # 应看到 my_math.h
🔗 三、使用已安装的库（外部项目）
创建独立项目 day4_use_installed，链接之前安装好的 my_math。

1. 目录结构
text
day4_use_installed/
├── CMakeLists.txt
└── main.cpp
2. main.cpp
cpp
#include <iostream>
#include <my_math/my_math.h>

int main() {
    std::cout << "Using installed library: 100 + 200 = " << add(100, 200) << std::endl;
    return 0;
}
3. CMakeLists.txt（使用绝对路径）
cmake
cmake_minimum_required(VERSION 3.10)
project(UseInstalled)

add_executable(use_installed main.cpp)

target_link_libraries(use_installed PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../day4/install/lib/libmy_math.a
)
target_include_directories(use_installed PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../day4/install/include
)
4. 构建运行
bash
cd ~/cpp-ai-learning/day4_use_installed
mkdir build && cd build
cmake ..
make
./use_installed
输出：

text
Using installed library: 100 + 200 = 300
💡 提示：也可以使用 find_library 方式，更专业：

cmake
find_library(MY_MATH_LIB my_math PATHS ../day4/install/lib NO_DEFAULT_PATH)
target_link_libraries(use_installed PRIVATE ${MY_MATH_LIB})
🧠 关键概念回顾
概念	说明	命令/语法
单元测试	验证代码片段功能的自动化程序	enable_testing(), add_test()
安装规则	将构建产物复制到指定目录（如系统或打包目录）	install(TARGETS ... DESTINATION ...)
安装前缀	安装的根目录，可用 CMAKE_INSTALL_PREFIX 控制	cmake .. -DCMAKE_INSTALL_PREFIX=./install
链接已安装库	外部项目通过绝对路径或 find_library 找到 .a 文件并链接	target_link_libraries(... /path/to/libxxx.a)
❓ 今日遇到的问题与解决
问题 1：FetchContent 下载 fmt 失败（网络连接被重置）
解决：暂时注释掉相关代码，先专注核心 CMake 功能。以后网络稳定了再启用。

问题 2：外部项目链接库时出现 找不到 -l../day4/install/lib/libmy_math.a
解决：CMake 中相对路径传给链接器会出错。改用绝对路径（${CMAKE_CURRENT_SOURCE_DIR}/../day4/install/lib/libmy_math.a）或 find_library 解决。

✅ 今日完成清单
成功绕过网络问题，构建纯本地项目

为 my_math 库编写并运行了单元测试

使用 install 将库和头文件安装到自定义目录

创建外部项目，链接并使用安装后的库

将所有代码和笔记推送到 GitHub 仓库

📚 扩展思考
如何将 my_math 编译成动态库（SHARED）？安装时的 LIBRARY DESTINATION 与 ARCHIVE DESTINATION 有何不同？

如何编写 CMake 的 Config.cmake 文件，使得 find_package(my_math) 能够直接工作？

如果要把单元测试框架换成 GoogleTest 或 Catch2，CMake 应该怎样集成？

我们来详细分析一下 install 命令中的各种 DESTINATION 类型，以及你笔记中提到的三个扩展思考任务。

📦 一、install(TARGETS ... DESTINATION ...) 详解
当你安装一个目标（库或可执行文件）时，CMake 根据目标类型自动区分安装到哪个 DESTINATION。常用类型有：

目标类型	对应的 DESTINATION 关键字	典型路径（默认）	说明
可执行文件	RUNTIME	bin	Windows 上为 .exe，Linux 上就是可执行文件
静态库 (.a)	ARCHIVE	lib 或 lib64	静态库文件
动态库 (.so)	LIBRARY	lib	运行时链接的库文件（Linux/macOS）
动态库的导出符号（Windows .dll）	RUNTIME	bin	Windows 上的 DLL 文件放在 RUNTIME，而导入库放在 ARCHIVE
公共头文件	使用 FILES 或 DIRECTORY 单独指定	include	不属 TARGETS，需单独用 install(FILES ... DESTINATION ...)
📌 实际例子（来自你的项目）
cmake
install(TARGETS my_math
        ARCHIVE DESTINATION lib   # 静态库 libmy_math.a 安装到 lib/
        LIBRARY DESTINATION lib   # 动态库 libmy_math.so 安装到 lib/
        RUNTIME DESTINATION bin   # 如果可执行文件，会安装到 bin/
)
如果同时构建静态库和动态库（通过 add_library(my_math STATIC ...) 和 add_library(my_math SHARED ...)），ARCHIVE 控制静态库，LIBRARY 控制动态库。

补充：DESTINATION 可以是绝对或相对路径
相对路径会基于 CMAKE_INSTALL_PREFIX 拼接。

例如 DESTINATION lib 且 -DCMAKE_INSTALL_PREFIX=/usr/local，则实际安装到 /usr/local/lib。

🧩 二、扩展任务分析
你的笔记中提到了三个扩展思考，我来逐一给出分析思路和可操作的步骤。

1. 如何将 my_math 编译成动态库（SHARED）？
分析：动态库在运行时加载，可以多个程序共享同一份库文件，减小可执行文件体积。但与静态库不同，你需要确保库文件在运行时可以被找到（例如设置 LD_LIBRARY_PATH 或安装到系统路径）。

操作步骤：

bash
cd ~/cpp-ai-learning/day4
cp -r math_lib math_lib_shared
修改 math_lib_shared/CMakeLists.txt：

cmake
add_library(my_math SHARED my_math.cpp)   # 将 STATIC 改为 SHARED
然后在顶层 CMakeLists.txt 中将其加入（可以同时保留静态库版本，或替换）。重新构建，你会得到 libmy_math.so。

安装规则不变：install(TARGETS my_math LIBRARY DESTINATION lib) 会安装动态库。

使用动态库的注意事项：

编译时需要链接器找到 .so 文件（与静态库相同）。

运行时，系统需要知道在哪里找到 .so。临时测试可以设置：

bash
export LD_LIBRARY_PATH=/path/to/install/lib:$LD_LIBRARY_PATH
更永久的方法是将库安装到系统标准路径（如 /usr/local/lib）并运行 ldconfig。

2. 如何编写 CMake 的 Config.cmake 文件，使得 find_package(my_math) 能够直接工作？
分析：CMake 的 find_package 有两种模式：模块模式（找 Find<package>.cmake）和配置模式（找 <package>Config.cmake）。自己写的库通常提供 <name>Config.cmake 文件，这样外部项目只需 find_package(my_math) 就能自动获得头文件路径、库路径和依赖。

实现思路（使用 CMake 自带的 write_basic_package_version_file 和 install 导出目标）：

步骤 1：在 math_lib/CMakeLists.txt 末尾添加导出配置
cmake
# 1. 导出 my_math 目标，供外部使用
install(TARGETS my_math
        EXPORT my_mathTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include
)

# 2. 安装导出文件 my_mathTargets.cmake
install(EXPORT my_mathTargets
        FILE my_mathTargets.cmake
        DESTINATION lib/cmake/my_math
)

# 3. 编写 Config.cmake 和 ConfigVersion.cmake 文件
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/my_mathConfig.cmake
    INSTALL_DESTINATION lib/cmake/my_math
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/my_mathConfigVersion.cmake
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion
)

# 4. 安装这两个文件
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/my_mathConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/my_mathConfigVersion.cmake
        DESTINATION lib/cmake/my_math
)
步骤 2：创建 Config.cmake.in 模板文件（放在 math_lib/ 目录下）
cmake
@PACKAGE_INIT@

include("${CMAKE_CURRENT_LIST_DIR}/my_mathTargets.cmake")

# 可选：设置变量供用户使用
set(MY_MATH_LIBRARIES my_math)
set(MY_MATH_INCLUDE_DIRS "${PACKAGE_PREFIX_DIR}/include")
步骤 3：构建并安装
bash
cd build
cmake ..
make
make install   # 会生成并安装 Config.cmake 文件
步骤 4：外部项目使用
cmake
find_package(my_math REQUIRED)
target_link_libraries(use_installed PRIVATE my_math::my_math)
这样你的库就拥有了和 OpenCV、fmt 一样的优雅体验。

3. 如何集成 GoogleTest 或 Catch2？
分析：现代 C++ 项目通常使用专门的测试框架，而不是手写断言。CMake 从 3.14+ 开始内置了对 GoogleTest 的支持。

选择 GoogleTest 为例：

方法一：使用 FetchContent（自动下载）
在顶层 CMakeLists.txt 中添加：

cmake
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(test_math tests/test_math.cpp)
target_link_libraries(test_math PRIVATE my_math gtest_main)
add_test(NAME MathTest COMMAND test_math)
然后修改 tests/test_math.cpp 使用 GoogleTest：

cpp
#include <gtest/gtest.h>
#include "my_math.h"

TEST(MathTest, Add) {
    EXPECT_EQ(add(2, 3), 5);
}

TEST(MathTest, Subtract) {
    EXPECT_EQ(subtract(5, 3), 2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
方法二：系统安装 GoogleTest（sudo apt install libgtest-dev 然后手动编译）
比较麻烦，推荐使用 FetchContent 或 vcpkg。

对于 Catch2（更轻量，仅头文件）：
cmake
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

target_link_libraries(test_math PRIVATE my_math Catch2::Catch2WithMain)
测试代码示例：

cpp
#include <catch2/catch_test_macros.hpp>
#include "my_math.h"

TEST_CASE("Addition works", "[math]") {
    REQUIRE(add(2, 3) == 5);
}
🎯 总结
扩展任务	核心知识点	难度
动态库	SHARED 关键字、运行时库路径（LD_LIBRARY_PATH）	⭐⭐
Config.cmake	导出目标、configure_package_config_file、write_basic_package_version_file	⭐⭐⭐⭐
测试框架	FetchContent 集成 GoogleTest/Catch2、改写测试用例	⭐⭐⭐
建议你按顺序尝试：先把动态库跑通，再玩一玩 GoogleTest 集成，最后挑战 Config.cmake（这确实需要一些 CMake 功力，但一旦掌握，你的库就具备了专业级交付能力）。

如果需要我针对其中某一个任务写出完整的可执行代码（例如完整的 Config.cmake 示例工程），请告诉我，我可以帮你生成一个独立的模板项目。
