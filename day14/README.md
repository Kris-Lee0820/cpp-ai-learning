1. C++异步四剑客
组件	角色	适用场景
std::async	最便捷的异步启动器，自动创建线程执行函数	快速执行异步函数、获取结果
std::future	异步结果的接收器，获取线程执行的返回值	所有需要获取异步结果的场景
std::packaged_task	任务包装器，把函数打包成异步任务，绑定 future	线程池、任务队列等需要预封装任务的场景
std::promise	值生产者，手动设置结果，通过 future 传递给其他线程	跨线程数据同步
2. std::future 的三种状态查询
cpp
std::future_status status = future.wait_for(std::chrono::milliseconds(100));
// status 可能为：deferred（未执行）、ready（已完成）、timeout（执行超时）
3. std::async 的两种启动策略
策略	说明
std::launch::async	立即创建新线程执行（默认）
std::launch::deferred	延迟到调用 get()/wait() 时才执行
4. std::shared_future：多次获取结果
future 的 get() 只能调用一次，如需多次获取，可使用 shared_future：

cpp
std::shared_future<int> shared = future.share();
int val1 = shared.get();  // 可以多次调用
int val2 = shared.get();

Day 14 完整学习日记：C++异步编程进阶 —— future、promise、packaged_task 与任务流水线
📌 今日学习内容概览
理论深入：std::async、std::future、std::packaged_task、std::promise 的原理与使用场景。

实践任务：

std::async + std::future 的基础异步执行与状态查询。
std::packaged_task 封装函数并传递到线程执行。
std::promise + std::future 跨线程数据同步。
基于 packaged_task 的任务队列 + 线程池实现。
扩展挑战：

实现 future 的链式调用（模拟 .then()）。
构建异步图像处理流水线（读取 → 处理 → 保存）。
使用 shared_future 广播结果到多个消费者。
📚 第一部分：理论基础
1. C++异步四剑客
组件	角色	适用场景
std::async	最便捷的异步启动器	快速执行异步函数、获取结果
std::future	异步结果的接收器	获取异步操作的返回值
std::packaged_task	任务包装器，绑定 future	线程池、任务队列
std::promise	值生产者，手动设置结果	跨线程数据同步
2. std::future 状态查询
wait_for() 返回 std::future_status::ready / timeout / deferred。

get() 只能调用一次，否则抛出异常。

3. std::shared_future
允许多次 get()，适合一对多的广播场景。

通过 future.share() 或 std::shared_future<T>(std::move(future)) 创建。

4. 启动策略
std::launch::async：立即创建线程执行。

std::launch::deferred：延迟到 get() / wait() 时在当前线程同步执行。

💻 第二部分：实践任务实现
任务一：std::async 基础用法与状态查询
代码 async_demo.cpp

异步执行计算任务，主线程可做其他操作。

使用 wait_for 查询任务是否完成，超时或延迟。

通过 get() 获取结果。

关键输出：

text
主线程 ID: 123456
子线程 ID: 123457 开始计算...
主线程可以执行其他操作...
任务仍在执行中...
计算结果: 100
任务二：std::packaged_task 封装任务
代码 packaged_task_demo.cpp

将函数 add_num 封装为 packaged_task<int(int,int)>。

获取 future 后，将任务移动到子线程执行。

主线程等待结果。

关键输出：

text
主线程 ID: 123456
主线程继续执行...
子线程 ID: 123457
10 + 20 = 30
任务三：std::promise + std::future 数据同步
代码 promise_demo.cpp

子线程通过 promise 设置值，主线程通过 future 等待接收。

演示了跨线程手动传递结果的方式。

关键输出：

text
主线程: 等待子线程计算结果...
子线程: 已设置值 25
主线程: 收到结果 = 25
任务四：基于 packaged_task 的任务队列 + 线程池
代码 task_queue_threadpool.cpp

实现 TaskQueue 类：内部使用 std::queue<packaged_task<...>> 存储任务，带互斥锁和条件变量。

实现 ThreadPool 类：多个工作线程从队列取任务执行。

用户通过 submit 提交任务，返回 future。

关键输出：

text
提交 10 个斐波那契计算任务...
所有任务已提交，等待结果...
fibonacci(30) = 832040
...
fibonacci(39) = 63245986
所有任务完成！
🧪 第三部分：扩展挑战实现
扩展挑战 1：future 链式调用（模拟 .then()）
代码 future_then_demo.cpp

实现模板函数 then，接收 future 和函数，返回新的 future。

内部使用 std::promise 和分离线程，完成链式传递。

演示了读取 → 灰度化 → 缩放 → 保存的异步链。

关键输出：

text
读取图像: test.jpg (子线程 123457)
灰度化处理 (子线程 123458)
缩放图像 (子线程 123459)
保存图像，最终数据大小: 100
=== 处理完成，最终结果: 100 ===
扩展挑战 2：异步图像处理流水线
代码 image_pipeline_async.cpp

三个阶段：读取图像（异步）、处理（CLAHE增强）、保存。

使用 std::async 构建依赖链：读取 → 处理 → 保存，并传递 future 依赖。

集成 OpenCV，处理真实图片，输出增强后的图像。

运行示例：

bash
./image_pipeline_async test.jpg result.jpg
=== 异步图像处理流水线 ===
输入: test.jpg
输出: result.jpg
[读取] test.jpg (线程 123457)
[处理] 图像尺寸 640x480 (线程 123458)
[保存] result.jpg (线程 123459)
=== 处理完成！耗时 0.415 秒 ===
扩展挑战 3：shared_future 广播结果
代码 shared_future_demo.cpp

生产者计算一个值，转换为 shared_future。

启动三个消费者线程，各自调用 shared_future.get() 获取结果并处理（打印、平方、奇偶判断）。

主线程也获取结果，演示多消费者共享。

关键输出：

text
生产者: 计算结果 = 42
消费者1: 收到值 = 42
消费者2: 收到值 = 42, 平方 = 1764
消费者3: 收到值 = 42, 是 偶数
主线程: 也收到了值 = 42
❌ 遇到的问题与解决方案
问题	原因	解决方案
future.get() 只能调用一次	future 移动语义，get() 会移动结果	使用 shared_future 支持多次 get()
packaged_task 不可拷贝	任务对象只能移动	使用 std::move 转移所有权
任务队列中 packaged_task 类型不统一	参数和返回值不同	使用 std::function 包装成 void() 任务，或使用模板
链式调用中任务依赖未正确传递	future 作为参数传递顺序错误	在 then 中使用 std::future<T>& 并提前 get()
OpenCV 链接错误	未指定 pkg-config	编译时添加 `pkg-config --cflags --libs opencv4`
📊 性能对比与总结
组件	执行方式	适用场景
std::async	最简洁，自动管理线程	简单的异步任务
packaged_task + 线程	可控，任务可复用	需要手动管理线程生命周期
任务队列 + 线程池	高并发，资源复用	大量短任务
链式调用	任务依赖明确	多阶段流水线
shared_future	一对多广播	结果共享场景
✅ 今日成果清单
掌握 std::async、std::future 基础用法与状态查询。

熟练使用 std::packaged_task 封装任务并绑定 future。

理解 std::promise 的手动值传递机制。

实现基于 packaged_task 的任务队列 + 线程池。

实现 future 链式调用（模拟 .then()）。

构建真实的异步图像处理流水线（读取 → 处理 → 保存）。

使用 shared_future 实现结果广播。

所有代码编译运行无误，性能符合预期。

📖 学习心得
packaged_task 与线程池是实际工程中常用的模式，它将任务定义与执行分离，便于管理和调度。

链式调用虽然 C++17 无原生支持，但通过手动实现能加深对 future 和 promise 工作原理的理解。

shared_future 在需要多消费者共用结果时非常有用，避免了重复计算。

异步图像处理流水线展示了如何将 I/O 和 CPU 密集操作并行，有效提升整体吞吐量。
