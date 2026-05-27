# Day 3 (2026-05-28) 学习笔记

## 今日内容
1.  **模块化构建**：使用 `add_subdirectory()` 组织多模块项目。
2.  **创建静态库**：使用 `add_library(math STATIC ...)` 和 `target_include_directories(... PUBLIC ...)` 创建并暴露库的头文件。
3.  **链接库**：使用 `target_link_libraries(app PRIVATE math)` 链接依赖。
4.  **引入第三方库**：学习了使用 `FetchContent` 自动管理和引入 `fmt` 库。

## 关键命令回顾
```bash
# 构建
mkdir build && cd build
cmake ..
make
# 运行
./app/day3_app

==================================================================================
📅 Day 3 详细任务清单（2026-05-28）
🎯 今日核心目标
掌握模块化思想：理解为什么需要将代码拆分为库（.a 或 .so）和主程序。

学会创建与使用静态库：使用 add_library 命令，这是构建大型项目的基础能力。

探索 CMake 引入第三方库：掌握两种主流方式，为后续集成AI框架做准备。

📚 第一部分：理论知识预习（10分钟）
在进行今天的实践前，花几分钟了解核心概念会让你的操作更有方向感：

模块化 (Modularization)：将不同功能的代码（例如：数学计算、图像处理）放在独立的文件中，编译成库，这样主程序调用时逻辑会更清晰，修改时也互不影响。

静态库 (.a)：在编译时，库的代码会被直接“复制”到你的最终程序里，优势是发布时无需携带额外的库文件。

动态库 (.so)：程序运行时才去加载它，优势是文件体积小，且多个程序可以共享同一个库文件。

核心CMake命令：今天的核心是掌握 add_library、target_include_directories、target_link_libraries 这几个命令的组合用法。

💻 第二部分：动手实践——构建模块化C++项目（预计45分钟）
我们将一步步构建一个包含静态库模块和可执行主程序的项目。

步骤 1：构建项目目录结构
一个清晰的目录结构是良好工程实践的开始。请在终端中执行以下命令来搭建我们今天的“施工场地”：

bash
cd ~/cpp-ai-learning
mkdir -p day3/math_lib day3/app
cd day3
最终目录结构预览：

text
~/cpp-ai-learning/day3/
├── CMakeLists.txt       # 顶层的CMake文件
├── math_lib/            # 我们将创建的静态库模块，负责数学运算
│   ├── CMakeLists.txt
│   ├── my_math.h
│   └── my_math.cpp
└── app/                 # 主程序模块，调用数学库
    ├── CMakeLists.txt
    └── main.cpp
步骤 2：编写静态库（math_lib）的代码
在 math_lib 目录下，依次创建以下三个文件。

头文件 (math_lib/my_math.h)：声明两个简单的数学函数。

bash
nano math_lib/my_math.h
输入以下内容：

cpp
#ifndef MY_MATH_H
#define MY_MATH_H

int add(int a, int b);
int subtract(int a, int b);

#endif // MY_MATH_H
保存退出。

源文件 (math_lib/my_math.cpp)：实现头文件中声明的函数。

bash
nano math_lib/my_math.cpp
输入以下内容：

cpp
#include "my_math.h"

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}
保存退出。

为静态库编写 CMakeLists.txt (math_lib/CMakeLists.txt)：

bash
nano math_lib/CMakeLists.txt
输入以下内容。这里使用 STATIC 关键字告诉 CMake 我们构建的是静态库。

cmake
cmake_minimum_required(VERSION 3.10)
project(MathLibrary)

# 创建一个名为 my_math 的静态库
add_library(my_math STATIC my_math.cpp)

# 让使用 my_math 库的目标能自动找到头文件所在目录
target_include_directories(my_math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
保存退出。

步骤 3：编写主程序（app）的代码
app 模块将调用 math_lib 库中的函数。

主程序源文件 (app/main.cpp)：

bash
nano app/main.cpp
输入以下内容：

cpp
#include <iostream>
#include "my_math.h" // 包含我们的数学库头文件

int main() {
    int a = 10, b = 5;
    std::cout << "CMake Day 3: 模块化构建成功！" << std::endl;
    std::cout << a << " + " << b << " = " << add(a, b) << std::endl;
    std::cout << a << " - " << b << " = " << subtract(a, b) << std::endl;
    return 0;
}
保存退出。

为主程序编写 CMakeLists.txt (app/CMakeLists.txt)：

bash
nano app/CMakeLists.txt
输入以下内容：

cmake
cmake_minimum_required(VERSION 3.10)
project(MyApp)

# 创建一个可执行文件，目标名为 day3_app
add_executable(day3_app main.cpp)

# 将我们的数学库链接到这个可执行文件
target_link_libraries(day3_app PRIVATE my_math)
保存退出。

步骤 4：编写顶层 CMakeLists.txt
这是整个项目的“总指挥”，它负责将 math_lib 和 app 这两个模块组织起来。

bash
nano CMakeLists.txt
输入以下内容：

cmake
cmake_minimum_required(VERSION 3.10)
project(ModularProject)

# 设置C++标准为C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加子目录，这里的顺序很重要：app 模块依赖于 math_lib 模块
add_subdirectory(math_lib)
add_subdirectory(app)
保存退出。

步骤 5：构建整个项目
进行标准的“外部构建”，这是我们在Day 2就已熟悉的最佳实践。

bash
# 确保你在 ~/cpp-ai-learning/day3 目录下
mkdir build && cd build
cmake ..
make
如果一切顺利，在 build/app 目录下，你会看到 day3_app 这个可执行文件。

步骤 6：运行你的程序
bash
./app/day3_app
预期输出：

text
CMake Day 3: 模块化构建成功！
10 + 5 = 15
10 - 5 = 5
看到这个结果，就意味着你成功构建了一个模块化的C++项目！🎉

🛠️ 第三部分：探究构建细节（15分钟）
构建成功后，花点时间深入探究一下，这将大大加深你的理解。

剖析 build 目录：

bash
ls -l build/
ls -l build/math_lib/
ls -l build/app/
math_lib 目录下会生成一个 libmy_math.a 文件，这是我们的静态库。

app 目录下是最终的可执行文件和它编译过程中的中间文件。

尝试修改静态库：

打开 math_lib/my_math.cpp，新增一个乘法函数 multiply(int a, int b)，记得在 my_math.h 中也要添加声明。

在 app/main.cpp 中调用 multiply()。

进入 build 目录，再次运行 make。你会发现CMake会自动检测到源代码变化，并只重新编译被修改的库和依赖它的主程序，体现了大型项目增量编译的高效。

🚀 第四部分：挑战与拓展——引入第三方库（选做/20分钟）
在真实开发中，我们经常会用到像 fmt 这样的第三方库。CMake 提供了优雅的方式来管理它们，主要有两种主流方法。

方法一：现代推荐 FetchContent （自动下载）
这种方法最适合开源或个人项目，它能自动从 GitHub 等仓库下载并编译依赖。

修改顶层 CMakeLists.txt：

bash
nano CMakeLists.txt
将文件内容替换为以下内容：

cmake
cmake_minimum_required(VERSION 3.14) # FetchContent 需要稍高的版本
project(ModularProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 引入 FetchContent 模块[reference:4]
include(FetchContent)

# 声明我们要获取 fmt 库[reference:5]
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.1.0      # 使用一个稳定的版本号
)
FetchContent_MakeAvailable(fmt)

add_subdirectory(math_lib)
add_subdirectory(app)
保存退出。

修改主程序 app/main.cpp，使用 fmt 库进行格式化输出：

cpp
#include <iostream>
#include "my_math.h"
#include <fmt/core.h> // 包含 fmt 库的头文件

int main() {
    int a = 10, b = 5;
    // 使用 fmt 库进行格式化输出
    fmt::print("CMake Day 3 挑战: 使用第三方库 {}\n", "fmt");
    std::cout << a << " + " << b << " = " << add(a, b) << std::endl;
    std::cout << a << " - " << b << " = " << subtract(a, b) << std::endl;
    return 0;
}
保存退出。

修改 app/CMakeLists.txt，链接 fmt 库。

cmake
cmake_minimum_required(VERSION 3.10)
project(MyApp)

add_executable(day3_app main.cpp)
# 同时链接我们的数学库和 fmt 库
target_link_libraries(day3_app PRIVATE my_math fmt::fmt)
保存退出。

重新构建：

bash
# 为了干净的环境，建议删除 build 目录，让它重新配置
cd ~/cpp-ai-learning/day3
rm -rf build
mkdir build && cd build
cmake ..
make
这次 cmake .. 命令执行时，你会看到它正在自动下载 fmt 库，这是正常现象。

运行程序：

bash
./app/day3_app
如果看到 CMake Day 3 挑战: 使用第三方库 fmt 的输出，恭喜你，你成功地在项目中集成了外部依赖！

方法二：传统方法 find_package （备选）
这种方法适用于库已经安装在系统路径中的情况，例如系统通过 apt 安装的 OpenCV、Boost 等。它的核心是 find_package(OpenCV REQUIRED) 和后续的 target_link_libraries(... ${OpenCV_LIBS})。

✅ 今日完成检查清单
我能够手写出创建一个静态库的 CMakeLists.txt。

我能理解 target_include_directories(... PUBLIC ...) 的作用。

我成功构建并运行了一个包含多个模块的项目。

我理解了 FetchContent 在项目中自动引入第三方库的流程。

我将所有代码和笔记推送到 GitHub（别忘了记录今天的输出和命令）。

完成“探究构建细节”中的步骤。

❓ 常见问题与解决
Could not find a package configuration file provided by "fmt"：如果 FetchContent 下载失败，通常是网络问题。可以尝试换个网络环境，或手动下载 fmt 源码并放到 day3/third_party/fmt 目录下，然后用 add_subdirectory(third_party/fmt) 方式引入。

undefined reference to 'add(int, int)'：这表示链接步骤出了问题，最常见的原因是忘记在 app/CMakeLists.txt 中调用 target_link_libraries。请检查你的 target_link_libraries(day3_app PRIVATE my_math) 是否拼写正确。

cannot open source file "fmt/core.h"：通常是因为没有在 app/CMakeLists.txt 中链接 fmt 库。请确保你添加了 target_link_libraries(day3_app PRIVATE my_math fmt::fmt)。










