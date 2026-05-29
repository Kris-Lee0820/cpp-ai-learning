# Day 6 (2026-05-29) 学习日记：CMake条件编译 + OpenCV视频处理与交互

## 📌 今日学习内容概览
- **基础模块**：CMake 条件编译与生成器表达式（`$<CONFIG>`、`target_compile_definitions`）
- **进阶模块**：OpenCV 视频处理（摄像头/文件读取、实时亮度/对比度调节、Canny边缘检测）
- **交互增强**：多滑动条（亮度、对比度、Canny低/高阈值）、按键重置（`r`）、帧率显示（FPS）
- **问题修复**：视频循环播放、灰度视频通道转换（`ensureBGR`）、OpenCV 滑动条警告处理

---

## 🧱 第一部分：CMake 条件编译与生成器表达式（基础）

### 1. 背景：为什么需要条件编译？
在实际开发中，我们常常需要根据不同的构建类型（Debug / Release）或编译器来设置不同的编译选项：
- **Debug 模式**：需要调试符号 `-g`、关闭优化 `-O0`、定义 `DEBUG` 宏。
- **Release 模式**：需要高优化 `-O3`、定义 `NDEBUG` 关闭断言。

CMake 提供了两种主流方法：**传统 `if` 判断** 和 **生成器表达式**。

### 2. 传统方法：使用 `if` 判断 `CMAKE_BUILD_TYPE`

**示例 `CMakeLists.txt`**：
```cmake
cmake_minimum_required(VERSION 3.10)
project(Day6Basics)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(day6_app main.cpp)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(day6_app PRIVATE DEBUG_MODE)
    target_compile_options(day6_app PRIVATE -g -O0)
else()
    target_compile_definitions(day6_app PRIVATE NDEBUG)
    target_compile_options(day6_app PRIVATE -O3)
endif()
测试命令：

bash
mkdir build && cd build
cmake ..                     # 默认 Debug
make
./day6_app                   # 输出 "Debug mode"

cmake .. -DCMAKE_BUILD_TYPE=Release
make clean && make
./day6_app                   # 输出 "Release mode"
3. 现代方法：生成器表达式（Generator Expressions）
生成器表达式在构建系统生成时求值，支持多配置生成器（如 Visual Studio），更加灵活。

常用表达式：

$<CONFIG>：当前配置名（Debug / Release）

$<COMPILE_LANGUAGE>：编译语言（CXX / C）

$<IF:cond,val_true,val_false>：条件选择

改造后的 CMakeLists.txt：

cmake
cmake_minimum_required(VERSION 3.10)
project(Day6Basics)

add_executable(day6_app main.cpp)

target_compile_definitions(day6_app PRIVATE
    $<$<CONFIG:Debug>:DEBUG_MODE>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(day6_app PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)
优点：无需显式判断 CMAKE_BUILD_TYPE，代码更简洁，且对于多配置生成器一次配置即可。

4. 根据编译器类型添加警告标志（实用拓展）
cmake
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(day6_app PRIVATE -Wall -Wextra -Wpedantic)
endif()
5. 今日基础模块实践成果
创建了 day6_basics 目录，编写了 main.cpp 和 CMakeLists.txt。

成功切换 Debug/Release 并观察宏定义的变化。

理解了生成器表达式的含义和用法。

🚀 第二部分：OpenCV 视频处理（进阶）
1. 目标
从摄像头或视频文件读取实时流。

提供滑动条：亮度、对比度、边缘模式（原图/Canny）、Canny低阈值、Canny高阈值。

实时显示调整后的画面，并叠加帧率（FPS）。

支持视频循环播放（文件播放完后自动重头开始）。

按 r 键重置所有参数，按空格暂停，按 ESC 退出。

2. 核心技术点
2.1 视频捕获与循环播放
cpp
cv::VideoCapture cap;
if (argc > 1) cap.open(argv[1]);
else cap.open(0);  // 摄像头

while (true) {
    cap >> g_frame;
    if (g_frame.empty()) {
        if (argc > 1) {
            cap.set(cv::CAP_PROP_POS_FRAMES, 0);  // 重置到第一帧
            continue;
        } else break;
    }
    // 处理帧...
}
2.2 通道自适应（解决灰度视频崩溃问题）
由于某些视频是单通道（灰度），而 cvtColor(..., COLOR_BGR2GRAY) 需要三通道输入，我们编写 ensureBGR 函数：

cpp
cv::Mat ensureBGR(const cv::Mat& src) {
    if (src.channels() == 3) return src.clone();
    if (src.channels() == 1) {
        cv::Mat bgr;
        cv::cvtColor(src, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    // 处理 RGBA 等
    cv::Mat bgr;
    cv::cvtColor(src, bgr, cv::COLOR_BGRA2BGR);
    return bgr;
}
2.3 亮度和对比度调整
cpp
int brightness_offset = g_brightness - 50;   // -50..+50
double contrast_alpha = g_contrast / 100.0;  // 0.5..2.0
cv::Mat adjusted;
bgr_frame.convertTo(adjusted, -1, contrast_alpha, brightness_offset);
2.4 Canny 边缘检测（带动态阈值）
cpp
cv::Mat gray, edges;
cv::cvtColor(adjusted, gray, cv::COLOR_BGR2GRAY);
cv::Canny(gray, edges, g_canny_low, g_canny_high);
cv::cvtColor(edges, display, cv::COLOR_GRAY2BGR);
2.5 滑动条创建与回调
注意：为了实时响应，我们允许 OpenCV 的警告（传递指针），因为这样可以在回调中直接获取新值。

cpp
cv::createTrackbar("Brightness", "Video", &g_brightness, 100, onBrightnessChange);
cv::createTrackbar("Contrast",   "Video", &g_contrast,   200, onContrastChange);
cv::createTrackbar("Edge Mode",  "Video", &g_edge_mode,   1,  onEdgeModeChange);
cv::createTrackbar("Canny Low",  "Video", &g_canny_low,   255, onCannyLowChange);
cv::createTrackbar("Canny High", "Video", &g_canny_high,  255, onCannyHighChange);
每个回调函数调用 processFrame() 刷新显示。

2.6 帧率计算（平滑滤波）
cpp
int64 new_tick = cv::getTickCount();
double elapsed = (new_tick - g_tick) / cv::getTickFrequency();
g_fps = 0.9 * g_fps + 0.1 / elapsed;   // 低通滤波
g_tick = new_tick;
显示使用 cv::putText。

2.7 重置所有参数
cpp
void resetParameters() {
    g_brightness = 50; g_contrast = 100; g_edge_mode = 0;
    g_canny_low = 50; g_canny_high = 150;
    cv::setTrackbarPos("Brightness", "Video", g_brightness);
    // ... 其他滑动条
    processFrame();
}
在主循环中检测按键 'r' 调用。

3. 完整代码结构
全局变量（亮度、对比度、边缘模式、Canny阈值、帧、FPS计时）

ensureBGR 函数

滑动条回调函数

processFrame 核心处理（应用参数、生成显示图像、叠加FPS）

resetParameters 重置函数

main 函数：打开视频源、创建窗口和滑动条、主循环（读帧+处理+按键响应）

4. 编译与运行
bash
cd ~/cpp-ai-learning/day6_adv
rm -rf build && mkdir build && cd build
cmake ..
make
./video_demo ../test.avi   # 或 ./video_demo（摄像头）
❌ 遇到的困难与解决方案
问题 1：视频播放一次就退出
原因：当 cap >> frame 返回空时直接跳出循环。

解决：检测到空帧后，判断是否为文件（argc > 1），若是则调用 cap.set(cv::CAP_PROP_POS_FRAMES, 0) 重置位置并继续循环。

问题 2：OpenCV 报错 Invalid number of channels ... scn is 1
原因：输入视频是灰度（单通道），而 cvtColor(..., COLOR_BGR2GRAY) 要求源图像是3通道。

解决：编写 ensureBGR 函数，在处理前统一转为 BGR 三通道。在 processFrame 中首先调用 cv::Mat bgr_frame = ensureBGR(g_frame);。

问题 3：createTrackbar 产生警告 Using 'value' pointer is unsafe
原因：传递指针方式在新版 OpenCV 中被标记为不安全。

处理：为了保留回调实时性，暂时忽略警告（不影响运行）。若想消除，可将指针参数设为 NULL，然后在主循环中用 getTrackbarPos 轮询，但那样会失去回调的即时性。

问题 4：帧率显示不流畅或为 0
原因：计时器初始化不当或未在每帧更新。

解决：使用 cv::getTickCount() 和 cv::getTickFrequency()，采用低通滤波平滑显示。

问题 5：重置参数后滑动条位置未同步
原因：仅修改了全局变量，未调用 cv::setTrackbarPos。

解决：在 resetParameters 中为每个滑动条设置位置。

🧪 扩展练习成果
为 Canny 阈值添加两个滑动条
新增全局变量 g_canny_low、g_canny_high，范围 0～255，回调中实时更新。在 processFrame 中使用动态阈值。

按 r 键重置所有参数
实现 resetParameters 函数，重置五个参数并将滑动条位置归位。

在图像左上角显示实时帧率
使用低通滤波平滑 FPS 值，用 cv::putText 绘制绿色文字。
