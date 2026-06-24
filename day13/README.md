Day 13 完整学习日记：条件变量、异步任务链与并发流水线
📌 今日学习内容概览
理论：条件变量（std::condition_variable）、std::future/std::promise、std::packaged_task。

实践任务：

实现有界阻塞队列（生产者-消费者模型）。
使用条件变量优化线程池（消除忙等）。
使用 std::promise / std::future 传递异步结果。
扩展挑战：

手动组合 future 构建图像处理链（灰度→缩放→显示）。
基于阻塞队列构建视频处理流水线（读取→处理→显示三阶段并行）。
📚 第一部分：理论基础
1. 条件变量（std::condition_variable）
用于线程间同步，允许线程阻塞等待某个条件成立。

必须与 std::unique_lock<std::mutex> 配合使用。

核心方法：

wait(lock, predicate)：释放锁并阻塞，直到谓词为 true。

notify_one()：唤醒一个等待线程。

notify_all()：唤醒所有等待线程。

优势：避免忙等（busy-wait），节省 CPU 资源。

2. std::future / std::promise / std::packaged_task
std::promise 用于设置值，std::future 用于获取值，两者通过共享状态关联。

std::packaged_task 封装可调用对象，自动提供 future，适合在线程池中异步执行。

组合能力：C++17 缺乏 .then()，但可通过连续 std::async 或显式等待实现链式操作。

3. 阻塞队列（Blocking Queue）
固定容量，当队列满时 push 阻塞，空时 pop 阻塞。

常用于生产者-消费者模型，控制内存使用，平衡生产消费速度。

使用条件变量实现，比循环检测更高效。

💻 第二部分：实践任务实现
任务一：有界阻塞队列
代码 BlockingQueue.hpp

包含 push（阻塞直到有空间）、pop（阻塞直到有元素）、try_push/try_pop（非阻塞）。

使用两个条件变量 not_empty 和 not_full，分别唤醒消费者和生产者。

测试：生产者写入 20 个数，消费者读取并输出，验证顺序与完整性。

关键代码：

cpp
void push(const T& item) {
    std::unique_lock<std::mutex> lock(mtx);
    not_full.wait(lock, [this] { return queue.size() < capacity; });
    queue.push(item);
    not_empty.notify_one();
}
任务二：条件变量优化的线程池
头文件 ThreadPool.hpp

使用 std::condition_variable 替代忙等。

工作线程循环：等待条件（有任务或停止），取出任务执行。

enqueue 添加任务后 notify_one，唤醒一个空闲线程。

析构时设置停止标志并唤醒所有线程，等待它们结束。

与忙等版本对比：CPU 占用显著降低，空闲时线程休眠。

测试：提交 20 个任务，每个任务模拟耗时 50ms，获取结果并输出。

任务三：std::promise 与 std::future 示例
代码 promise_future_demo.cpp

主线程创建 std::promise<cv::Mat>，获取 future。

工作线程读取图像并处理（灰度化），调用 promise.set_value(result)。

主线程调用 future.get() 阻塞等待结果并显示。

应用场景：解耦异步任务的产生和消费。

🧪 第三部分：扩展挑战实现
扩展挑战 1：异步任务链（手动组合 future）
由于 C++17 没有 .then()，我们采用显式等待方式，但保持每一步使用 std::async。

实现：

cpp
auto f1 = std::async(std::launch::async, readImage, "test.jpg");
auto img1 = f1.get();
auto f2 = std::async(std::launch::async, grayscale, img1);
auto img2 = f2.get();
auto f3 = std::async(std::launch::async, resizeImg, img2, 400,300);
auto img3 = f3.get();
showImg(img3);
虽然阻塞，但结构清晰，易于理解。

改进思路：可以使用 std::future 的扩展库（如 Folly）或手动构建任务图。

扩展挑战 2：基于阻塞队列的视频处理流水线
代码 video_pipeline_conditional.cpp

三个线程：读取帧、处理帧（高斯模糊）、显示帧。

使用两个阻塞队列：raw_queue（原始帧）和 processed_queue（处理后的帧）。

读取线程从摄像头/文件读取，推入 raw_queue。

处理线程从 raw_queue 取出，处理，推入 processed_queue。

显示线程从 processed_queue 取出并显示。

空帧作为结束信号，终止流水线。

效果：读、处理、显示并行，提高整体 FPS，尤其当处理耗时较长时。

❌ 遇到的问题与解决方案
问题	原因	解决方案
线程池析构时死锁	未正确通知条件变量	在析构中先 lock 设置 stop=true，再 notify_all()
push 阻塞后无法退出	未处理停止信号	增加 stop 标志并在条件中检查，支持强制退出
流水线中空帧信号传播	使用 cv::Mat() 作为结束标记	检查 frame.empty() 并传递空帧到下一阶段
任务链中异步任务顺序混乱	错误地启动多个异步任务	改为串行等待每个 future，确保执行顺序
📊 性能对比（忙等 vs 条件变量）
忙等版本：空闲时 CPU 占用 ~100%（单核）。

条件变量版本：空闲时 CPU 占用 ~0%，线程休眠。

任务执行时，两者性能相当，但条件变量方式更节省资源。

✅ 今日成果清单
理解条件变量原理并实现有界阻塞队列。

使用条件变量优化线程池，消除忙等，显著降低 CPU 占用。

掌握 std::promise / std::future 的基本用法。

实现异步任务链（手动组合）。

构建基于阻塞队列的视频处理流水线，实现并行读取/处理/显示。

所有代码编译运行无误，性能满足预期。

📖 学习心得
条件变量是线程同步的重要工具，正确使用可显著提高程序效率。

阻塞队列是生产者-消费者模型的经典实现，在多阶段流水线中尤为重要。

异步任务链（.then 风格）在 C++17 中不直接支持，但可通过辅助工具实现，或接受显式等待方式。

流水线并行通过解耦阶段，提高吞吐量，适合 I/O 密集型或计算密集型混合任务。
