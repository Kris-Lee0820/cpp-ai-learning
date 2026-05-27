 Day 2 详细任务清单（2026-05-27）
🎯 今日目标
理解 CMake 的基本作用（跨平台的构建工具）

学会编写 CMakeLists.txt 文件

用 CMake 构建并运行一个简单的 C++ 程序

将今天的项目上传到 GitHub（延续 Day 1 的仓库）

📚 第一部分：学习 CMake 基础概念（10分钟）
阅读内容：
CMake 是一个用来生成 Makefile 的工具，你只需要写一个 CMakeLists.txt，它就能自动适配不同编译器。

核心概念（先读一遍，实操中再巩固）：

cmake_minimum_required(VERSION 3.10)：指定所需 CMake 最低版本

project(项目名)：设置项目名称

add_executable(目标名 源文件1 源文件2...)：告诉 CMake 要生成一个可执行文件

set(变量名 值)：设置变量（比如 C++ 标准）

资料链接（可选，不用全部读完，边用边查）：

CMake 官方教程（英文） 只看前 2 节

中文快速入门：Modern CMake 简明教程 前 3 章

💻 第二部分：动手实践——第一个 CMake 项目（45分钟）
步骤 1：创建项目目录结构
bash
cd ~/cpp-ai-learning
mkdir -p day2/src
cd day2
现在你的目录结构如下：

text
~/cpp-ai-learning/
└── day2/
    └── src/
步骤 2：编写 C++ 源文件
bash
nano src/main.cpp
输入以下内容（一个简单的输出程序）：

cpp
#include <iostream>

int main() {
    std::cout << "Hello, CMake! This is Day 2." << std::endl;
    std::cout << "C++ version: " << __cplusplus << std::endl;
    return 0;
}
保存：Ctrl+O，回车，Ctrl+X 退出。

步骤 3：编写 CMakeLists.txt
bash
nano CMakeLists.txt
输入以下内容：

cmake
cmake_minimum_required(VERSION 3.10)
project(Day2Project)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(hello_day2 src/main.cpp)
保存退出。

步骤 4：使用 CMake 构建项目
重要：通常我们会在一个单独的 build 目录下构建，以保持源码目录整洁。

bash
# 在 day2 目录下创建 build 文件夹并进入
mkdir build
cd build

# 运行 CMake 配置（.. 表示上一级目录的 CMakeLists.txt）
cmake ..

# 编译项目
make
预期输出：

cmake .. 会显示 -- Configuring done 和 -- Generating done

make 会显示编译进度，最后生成 hello_day2 可执行文件

步骤 5：运行你的第一个 CMake 项目
bash
./hello_day2
预期输出：

text
Hello, CMake! This is Day 2.
C++ version: 201703
（__cplusplus 的值 201703 表示 C++17）

🔧 第三部分：探索与理解（20分钟）
任务 1：修改代码并重新编译
把输出文字改成你自己的名字或任何内容

修改 main.cpp，保存

在 build 目录下重新执行 make（CMake 会自动检测变化，只重新编译改动的文件）

再次运行 ./hello_day2 观察变化

任务 2：查看 CMake 生成的文件
bash
ls build/
你会看到 CMakeCache.txt、CMakeFiles 目录、Makefile 以及 hello_day2。
注意：这些文件都是自动生成的，不要手动修改。

任务 3：清理构建（可选）
bash
cd ~/cpp-ai-learning/day2/build
make clean   # 删除编译出的目标文件
或者直接删除整个 build 目录重新来：

bash
rm -rf build
mkdir build && cd build && cmake .. && make
📦 第四部分：记录笔记并推送到 GitHub（15分钟）
步骤 1：在 day2 目录下创建 README.md
bash
cd ~/cpp-ai-learning/day2
nano README.md
内容建议（包含你今天的命令和收获）：

markdown
# Day 2 (2026-05-27) 学习笔记

## 今日内容
- CMake 基础：编写 CMakeLists.txt，外部构建，编译运行 C++ 程序。
- 实践了从源码到可执行文件的完整流程。

## 关键命令
```bash
# 构建
mkdir build && cd build
cmake ..
make

# 运行
./hello_day2
CMakeLists.txt 示例
（粘贴你写的 CMakeLists.txt 内容）

遇到的问题与解决
（如果有报错，写在这里）

text

#### 步骤 2：提交到 Git
```bash
cd ~/cpp-ai-learning
git add day2/
git commit -m "Add day2: first CMake project with C++17"
git push
注意：如果你之前分支名是 master，使用 git push origin master；如果是 main，用 git push origin main。不确定的话执行 git branch 查看当前分支。

✅ 今日完成检查清单
能够手写一个最简单的 CMakeLists.txt

能够用 cmake .. 和 make 完成构建

理解源码目录和构建目录分离的好处

修改代码后只运行 make 就能增量编译

将 day2 的笔记和代码推送到 GitHub 仓库

（可选）尝试在 CMakeLists.txt 中添加第二个源文件（例如 utils.cpp），并修改 add_executable

❓ 常见问题与解决
Q: cmake 命令找不到？
A: 安装 CMake：sudo apt install cmake -y

Q: make 命令找不到？
A: 安装 make：sudo apt install make -y

Q: 编译时提示 C++ 标准不支持？
A: 确保 set(CMAKE_CXX_STANDARD 17) 写对了。如果编译器太旧（Ubuntu 18.04 默认 g++ 7），可以安装更新版本：sudo apt install g++-9，然后设置 CMAKE_CXX_COMPILER，不过一般 22.04 没问题。

Q: 推送时提示 Permission denied (publickey)？
A: 参考 Day 1 的 SSH 配置。可以先测试 ssh -T git@github.com，如果失败，运行 eval "$(ssh-agent -s)" 和 ssh-add ~/.ssh/id_ed25519 再试。

🚀 扩展挑战（学有余力）
在 CMakeLists.txt 中添加 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") 打开额外警告

创建一个 src/utils.cpp 和 src/utils.h，在 main.cpp 中调用，并修改 add_executable 包含两个源文件

📌 总结
今天你完成了从零到一使用 CMake 构建 C++ 项目。这是后续 AI 基础设施开发（如 ONNX Runtime、TensorRT 集成）的基础技能。把笔记推送到 GitHub 后，Day 2 就圆满结束。

如果你在执行中遇到任何报错，请把错误信息完整发给我，我会帮你解决。明天（Day 3）我们将学习 CMake 进阶：添加第三方库（如 OpenCV），为后续的模型推理做铺垫。
