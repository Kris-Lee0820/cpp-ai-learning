Day 9 (2026-06-02) 超详细学习日记：C++ 异步编程、懒加载与并行图像处理
📌 今日学习内容概览
基础模块：深入理解 std::async 的 deferred 与 async 策略；实现 Lazy<T> 类（延迟计算 + 结果缓存 + 隐式转换）。

进阶模块：std::future 的一次性语义；为 Lazy<T> 添加超时求值；将懒加载应用到人脸检测模型；使用 std::async 实现并行图像滤波。

工程实践：更新 CMakeLists.txt 支持 C++17 和 OpenCV；编写并行高斯模糊函数。

🧠 第一部分：std::async 启动策略对比
1. 三种策略的行为差异
策略	执行线程	执行时机	是否创建新线程
std::launch::async	新线程（或系统线程池）	立即	是（或复用）
std::launch::deferred	调用 get()/wait() 的线程	延迟到第一次调用 get()/wait()	否
默认（不指定）	由系统决定（通常是异步）	不确定	取决于实现
2. 验证 deferred 的延迟效果
测试代码 (deferred_demo.cpp)：

cpp
#include <iostream>
#include <future>
#include <chrono>
#include <thread>

int main() {
    auto deferred = std::async(std::launch::deferred, []{
        std::cout << "Task running...\n";
        return 42;
    });
    std::cout << "Task created, not executed yet\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Calling get()...\n";
    int val = deferred.get();
    std::cout << "Result: " << val << "\n";
    return 0;
}
输出：

text
Task created, not executed yet
(停顿 2 秒)
Calling get()...
Task running...
Result: 42
结论：deferred 任务不会立即执行，直到显式调用 get()/wait()。如果永远不调用，任务永远不会运行。

3. deferred 与同步调用的区别
相同点：都在当前线程同步执行。

不同点：deferred 可以控制执行时机，而直接调用函数会立即执行。deferred 适合条件计算（可能不需要结果时避免开销）。

🛠️ 第二部分：实现 Lazy<T> 类（懒加载 + 缓存）
2.1 基本要求
构造函数接受一个可调用对象（函数、lambda）。

第一次访问时执行计算并缓存结果，后续直接返回缓存值。

提供 get() 方法显式求值。

提供隐式转换，允许 Lazy<T> 当作 T 使用。

2.2 实现（使用 std::optional 缓存）
cpp
#include <functional>
#include <optional>

template<typename T>
class Lazy {
    std::function<T()> m_func;
    mutable std::optional<T> m_cache;
public:
    template<typename F>
    Lazy(F&& f) : m_func(std::forward<F>(f)) {}

    T get() const {
        if (!m_cache) m_cache = m_func();
        return *m_cache;
    }

    operator T() const {
        return get();
    }
};

// 辅助函数
template<typename F>
auto make_lazy(F&& f) -> Lazy<decltype(f())> {
    return Lazy<decltype(f())>(std::forward<F>(f));
}
2.3 测试代码
cpp
#include <iostream>
#include <chrono>
#include <thread>

int expensive() {
    std::cout << "Computing...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 42;
}

int main() {
    auto lazy = make_lazy(expensive);
    std::cout << "Lazy created, nothing computed\n";
    std::cout << "First get: " << lazy.get() << std::endl;
    std::cout << "Second get: " << lazy.get() << std::endl; // 不会重新计算
    return 0;
}
输出：

text
Lazy created, nothing computed
Computing...
First get: 42
Second get: 42
2.4 隐式转换的解释
隐式转换通过 operator T() 实现，允许 Lazy<T> 对象在需要 T 类型时自动求值：

cpp
Lazy<int> lazy_int = make_lazy([]() { return 100; });
int value = lazy_int;   // 自动调用 operator int()
使用场景：可以将 Lazy<T> 传递给接受 T 的函数，无需显式 .get()。

⚠️ 第三部分：std::future 的一次性语义与缓存必要性
3.1 future::get() 只能调用一次
cpp
std::promise<int> p;
std::future<int> f = p.get_future();
p.set_value(42);
std::cout << f.get() << std::endl;   // OK
// std::cout << f.get() << std::endl; // 抛出 std::future_error
std::cout << f.valid() << std::endl; // 输出 0
结论：std::future 本身不能作为缓存容器，必须自己保存计算结果（如 std::optional）。

3.2 错误尝试（使用 future 缓存）
cpp
template<typename T>
class Lazy_Bad {
    std::function<T()> m_func;
    std::future<T> m_future;
public:
    T get() {
        if (!m_future.valid())
            m_future = std::async(std::launch::deferred, m_func);
        return m_future.get();  // 第一次 get 后 future 失效
    }
};
问题：第二次调用 get() 时 m_future.valid() 为 false，会重新创建任务并再次计算，无法缓存结果。

正确做法：使用 std::optional 存储计算结果（如上所述）。

⏱️ 第四部分：为 Lazy<T> 添加超时求值
4.1 实现 get_for
cpp
template<typename Rep, typename Period>
T get_for(const std::chrono::duration<Rep, Period>& rel_time) {
    if (m_cache) return *m_cache;
    // 使用 async 启动异步任务（不受 deferred 限制）
    auto fut = std::async(std::launch::async, m_func);
    if (fut.wait_for(rel_time) == std::future_status::ready) {
        m_cache = fut.get();
        return *m_cache;
    } else {
        throw std::runtime_error("Computation timeout");
    }
}
4.2 测试
cpp
auto lazy = make_lazy([]{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 100;
});
try {
    int val = lazy.get_for(std::chrono::milliseconds(500));
} catch (const std::runtime_error& e) {
    std::cout << "Timeout: " << e.what() << std::endl;
}
输出：

text
Timeout: Computation timeout
🎯 第五部分：将懒加载应用到人脸检测模型
5.1 背景
原程序启动时立即加载 Haar 或 DNN 模型，导致启动缓慢。改为懒加载：只有首次按 f 键开启人脸检测时才加载模型。

5.2 实现（以 CascadeClassifier 为例）
cpp
// 全局懒加载对象
Lazy<cv::CascadeClassifier> lazy_face_cascade([]{ 
    cv::CascadeClassifier cascade;
    cascade.load("haarcascade_frontalface_default.xml");
    return cascade;
});

// 在 processFrame 中
if (g_face_detection_enabled) {
    const auto& face_cascade = lazy_face_cascade.get(); // 首次调用时加载
    // 使用 face_cascade.detectMultiScale(...)
}
5.3 验证
启动程序时观察是否加载模型（可通过日志或文件访问）。

按 f 键后才看到加载信息，之后检测正常。

⚡ 第六部分：并行图像滤波（进阶挑战）
6.1 任务要求
使用 std::async 将图像分割成多个水平条带，每个条带异步执行高斯模糊，最后等待所有任务完成。

6.2 完整实现
cpp
#include <opencv2/opencv.hpp>
#include <future>
#include <vector>

void blurRegion(cv::Mat& output, const cv::Mat& input, int start_row, int end_row, int kernel_size) {
    cv::Rect roi(0, start_row, input.cols, end_row - start_row);
    cv::GaussianBlur(input(roi), output(roi), cv::Size(kernel_size, kernel_size), 0);
}

void parallelGaussianBlur(const cv::Mat& src, cv::Mat& dst, int kernel_size, int num_segments = 4) {
    dst = src.clone();
    std::vector<std::future<void>> futures;
    int rows_per_segment = src.rows / num_segments;
    for (int i = 0; i < num_segments; ++i) {
        int start_row = i * rows_per_segment;
        int end_row = (i == num_segments - 1) ? src.rows : (i + 1) * rows_per_segment;
        futures.push_back(std::async(std::launch::async, blurRegion,
                                     std::ref(dst), std::ref(src),
                                     start_row, end_row, kernel_size));
    }
    for (auto& fut : futures) {
        if (fut.valid()) fut.wait();  // 等待每个任务完成
    }
}
6.3 测试
cpp
int main() {
    cv::Mat img = cv::imread("test.jpg");
    cv::Mat result;
    parallelGaussianBlur(img, result, 15, 4);
    cv::imshow("Original", img);
    cv::imshow("Parallel Blur", result);
    cv::waitKey(0);
    return 0;
}
6.4 关键点
std::future<void> 的 wait() 方法阻塞直到任务完成，无需 get()。

使用 std::ref 传递引用，避免拷贝图像数据。

每个任务写入不相交的行区间，无需加锁。

🛠️ 第七部分：更新 CMakeLists.txt
为支持 C++17 和 OpenCV，更新后的文件：

cmake
cmake_minimum_required(VERSION 3.10)
project(VideoDemo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
add_executable(video_demo video_demo.cpp)
target_link_libraries(video_demo ${OpenCV_LIBS})

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
📚 今日学习心得
std::async 的 deferred 策略：提供了延迟执行的能力，适合条件计算和懒加载。但与真正的异步不同，它不会创建线程，必须显式调用 get()/wait() 才会执行。

缓存的重要性：std::future 是一次性的，无法作为缓存容器。必须自己存储计算结果（如 std::optional）。

懒加载模式：在大型对象（如模型文件）初始化时非常有用，可以减少程序启动时间，按需加载。

并行图像处理：使用 std::async 可以轻松实现数据并行，但要注意任务粒度（过细会增加开销）。

隐式转换：提供了语法糖，但需谨慎使用（可能引发意外的计算）。

❌ 遇到的问题与解决
问题	原因	解决方案
第二次调用 Lazy::get() 仍重复计算	误用 std::future 缓存	改用 std::optional 存储结果
std::future<void> 不能调用 get() 但可以 wait()	标准库规定	使用 wait() 等待完成
并行滤波比单线程还慢	图像太小，线程创建开销 > 计算收益	仅对高分辨率图像（> 1080p）或复杂算法启用
模型加载导致程序启动慢	全局对象构造时加载	改用 Lazy 懒加载，首次使用时加载
📦 代码提交
所有代码已上传至 GitHub：

bash
cd ~/cpp-ai-learning
git add day9/
git commit -m "Day9: Lazy<T>, future semantics, parallel blur, lazy model loading"
git push
🕒 学习时长
约 3.5 小时（理论 + 编码 + 调试 + 文档）
