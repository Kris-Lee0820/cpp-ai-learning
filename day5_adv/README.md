# Day 5 (2026-05-28) 学习日记

## 今日学习内容概览
- **基础模块**：C++ 智能指针（`unique_ptr`、`shared_ptr`、`weak_ptr`）
- **进阶模块**：OpenCV 安装与第一个图像处理程序（读取、显示、灰度转换、缩放、保存）
- **工具链**：CMake 链接 OpenCV、Xshell 图形界面转发（Xmanager）

---

## 📚 第一部分：C++ 智能指针

### 学习要点
1. **为什么需要智能指针**  
   避免手动 `new`/`delete` 带来的内存泄漏和异常安全问题。

2. **`std::unique_ptr`**  
   - 独占所有权，不能拷贝，只能移动。  
   - 使用 `std::make_unique`（C++14）创建。  
   - 常用操作：`move`、`reset`、`release`。

3. **`std::shared_ptr`**  
   - 共享所有权，引用计数。  
   - 最后一个 `shared_ptr` 销毁时释放对象。  
   - 使用 `std::make_shared` 创建，`use_count()` 查看引用数。

4. **`std::weak_ptr`**  
   - 不增加引用计数，用于打破循环引用。  
   - 通过 `lock()` 获取临时的 `shared_ptr`。

5. **实践代码**（`smart_ptr.cpp`）  
   编译运行后观察到构造/析构的顺序，理解了所有权转移和引用计数。

### 遇到的问题
- 编译时需要指定 C++17 标准：`g++ -std=c++17 smart_ptr.cpp -o smart_ptr`。  
- 最初忘记包含 `<memory>` 头文件，导致编译失败。

---

## 🖼️ 第二部分：OpenCV 入门

### 安装 OpenCV
```bash
sudo apt update
sudo apt install libopencv-dev -y
验证：pkg-config --modversion opencv4 → 输出 4.5.4

编写第一个程序
功能：读取图片 → 显示原图 → 转灰度图并保存 → 缩放 → 显示缩放图。

关键代码：

cpp
cv::Mat img = cv::imread("test.jpg");
cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
cv::resize(img, resized, cv::Size(300, 300));
cv::imshow("Original", img);
cv::waitKey(0);
CMakeLists.txt 要点
cmake
find_package(OpenCV REQUIRED)
add_executable(opencv_demo main.cpp)
target_link_libraries(opencv_demo ${OpenCV_LIBS})
构建运行
bash
mkdir build && cd build
cmake ..
make
./opencv_demo
❌ 遇到的困难与解决
问题 1：imread 无法读取图片，报错 can't open/read file
原因：程序在 build 目录下运行，而图片 test.jpg 在上一级目录。

解决：

方法一：复制图片到 build 目录 cp ../test.jpg .

方法二：修改程序路径为 ../test.jpg 并重新编译。

方法三：在 CMakeLists.txt 中添加自动复制命令。

问题 2：图片文件损坏（只有 687 字节）
原因：使用 wget 下载链接失效或网络问题导致只下载了错误页面。

解决：重新下载有效图片，或手动上传一张正常图片。

bash
wget https://raw.githubusercontent.com/opencv/opencv/4.x/samples/data/lena.jpg -O test.jpg
验证文件大小应约为 70KB。

问题 3：在 Xshell 中运行程序无法显示图像窗口
原因：Xshell 是纯命令行终端，没有图形界面环境。

解决：

方案 A（推荐）：直接在 VMware 虚拟机的图形界面终端中运行程序。

方案 B：安装 Xmanager 并配置 X11 转发，将图形窗口显示到 Windows 桌面。

安装 Xmanager（官网下载 30 天试用版）。

在 Xshell 会话属性 → 隧道 → 勾选“转发 X11 连接到” → 选择“Xmanager”。

在 Ubuntu 中安装 xauth：sudo apt install xauth。

重新连接 SSH，运行 xeyes 测试，若弹出窗口则配置成功。

经过上述步骤，最终成功看到 OpenCV 显示的图像窗口。

📦 今日成果
掌握了三种智能指针的基本用法和适用场景。

成功安装 OpenCV 并编写了第一个图像处理程序。

理解了 CMake 中 find_package 查找第三方库的机制。

学会了解决 OpenCV 运行时图片路径问题和图形界面显示问题。

代码仓库
所有代码已提交到 GitHub：

day5_basics/smart_ptr.cpp

day5_adv/main.cpp、CMakeLists.txt

明日计划
基础：CMake 条件编译与生成器表达式。

进阶：OpenCV 视频处理（读取摄像头、实时显示、滑动条调节亮度）。

日记撰写时间：2026-05-28
学习时长：约 2 小时
心情：虽然有波折，但最终解决了所有问题，收获满满！
