Day 12 完整学习日记：C++ 内存模型、原子操作与无锁编程
📌 今日学习内容概览
理论部分：C++ 内存模型（顺序一致性、acquire-release、relaxed），原子类型与内存顺序。

实践任务：

使用 std::atomic 实现线程安全计数器，与 std::mutex 版本性能对比。
实现单生产者-单消费者无锁队列（基于循环数组）。
将原子停止标志集成到 Day11 的线程池中。
扩展挑战：

实现基于原子变量的读写锁（读者共享，写者独占）。
在并行图像处理中添加原子进度条，实时显示完成百分比。
📚 第一部分：理论基础回顾
1. 为什么需要内存模型？
多线程环境下，编译器和 CPU 可能对指令进行重排，导致数据竞争。

内存模型定义了线程间如何通过共享变量同步，确保可见性和顺序。

2. C++ 原子类型与内存顺序（重要）
内存顺序	保证	典型用法
memory_order_relaxed	仅原子性，无同步	计数器（不需与其他操作同步）
memory_order_acquire	之后的读写不能重排到 load 之前	读取锁或标志位
memory_order_release	之前的读写不能重排到 store 之后	设置锁或标志位
memory_order_acq_rel	同时具有 acquire 和 release 效果	RMW 操作（如 CAS）
memory_order_seq_cst	全局顺序一致性（默认）	通用，但开销最大
3. 常用原子操作
cpp
std::atomic<int> counter(0);
counter++;                               // fetch_add(1)
counter.fetch_add(1, std::memory_order_relaxed);
counter.load(std::memory_order_acquire);
counter.store(10, std::memory_order_release);
int expected = 5;
counter.compare_exchange_weak(expected, 10); // CAS
💻 第二部分：实践任务实现
任务一：原子计数器 vs 互斥锁
代码 atomic_counter.cpp

8 个线程，每个执行 1 亿次递增。

对比 std::atomic<long long> 和 std::mutex + long long 的性能。

运行结果（i7-9700K，8核）：

text
Atomic counter: 800000000  time: 0.823s
Mutex counter:   800000000  time: 6.471s
原子版本快了约 7.8 倍，无锁优势明显。

关键代码：

cpp
void atomicCounter(std::atomic<long long>& counter) {
    for (long long i = 0; i < ITERATIONS; ++i)
        counter.fetch_add(1, std::memory_order_relaxed);
}
使用 relaxed 因为不依赖其他操作，仅需原子性。

任务二：单生产者-单消费者无锁队列
头文件 lockfree_queue.hpp

基于循环数组，使用 std::atomic<size_t> 管理 head/tail。

支持 push 和 pop，返回 bool 表示成功/失败。

测试 test_lockfree_queue.cpp

生产者 push 10 万个整数，消费者累加。

验证结果正确性（总和 4999950000）。

性能：在无竞争下比 std::queue + mutex 快约 3 倍。

关键问题：消费者循环逻辑初版存在死循环（已在错误分析中修正），最终采用：

cpp
while (true) {
    if (queue.pop(value)) {
        sum += value;
    } else if (stop) {
        break;
    } else {
        std::this_thread::yield();
    }
}
任务三：线程池集成原子停止标志
修改 ThreadPool.hpp：

将 bool stop 替换为 std::atomic<bool> stop。

在 enqueue 检查 stop.load(std::memory_order_acquire)。

析构函数 stop.store(true, std::memory_order_release)。

工作线程循环条件使用 load。

好处：无需互斥锁，正确保证停止标志的可见性。

🧪 第三部分：扩展挑战实现
扩展挑战 1：读写锁（共享锁）
头文件 rw_lock.hpp

使用 std::atomic<int> reader_count、writer_waiting、writer_active。

写者优先策略：写者等待时会阻止新读者进入，避免饥饿。

自旋等待（while 循环 + yield()）。

测试 test_rw_lock.cpp：

3 个写者，10 个读者并发。

输出显示读者可并发读取，写者独占写入。

关键逻辑：

cpp
void write_lock() {
    writer_waiting.fetch_add(1, std::memory_order_release);
    while (reader_count.load(std::memory_order_acquire) > 0 ||
           writer_active.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    bool expected = false;
    while (!writer_active.compare_exchange_weak(expected, true,
                  std::memory_order_acq_rel, std::memory_order_acquire)) {
        expected = false;
        std::this_thread::yield();
    }
    writer_waiting.fetch_sub(1, std::memory_order_release);
}
使用 compare_exchange_weak 确保只有一个写者成功。

扩展挑战 2：图像处理进度条
代码 parallel_batch_with_progress.cpp

基于 Day11 的批量异步图像处理，添加 std::atomic<size_t> completed。

启动监控线程，每秒打印进度百分比。

主线程等待所有任务完成后停止监控。

效果：

text
Progress: 23.5% (47/200)
Progress: 51.0% (102/200)
...
All images processed in 12.34 seconds.
关键原子操作：

cpp
completed.fetch_add(1, std::memory_order_release);   // 任务完成时
size_t done = completed.load(std::memory_order_acquire); // 监控读取
❌ 遇到的问题与解决方案
问题	原因	解决方案
消费者死循环	while 条件中重复 pop 且未正确处理 stop	重构循环，先 pop 后判断 stop
读写锁写者饥饿	读者优先策略	引入 writer_waiting 计数器，写者等待时阻止新读者
原子计数器进度不更新	内存顺序使用不当	使用 release 存储，acquire 加载保证可见性
CAS 自旋失败	未重置 expected	在循环中每次设置 expected = false
📊 性能总结
组件	原子版本耗时	互斥版本耗时	加速比
计数器（8 线程，1e8 次）	0.823s	6.471s	7.86x
无锁队列（10 万次）	0.012s	0.041s	3.4x
读写锁（读者多）	读者无竞争	写者串行	显著
✅ 今日成果清单
理解四种内存顺序并应用到代码中。

实现并测试原子计数器与互斥锁版本，记录性能对比。

实现单生产者-单消费者无锁队列，验证正确性。

将 std::atomic<bool> 集成到线程池。

实现原子读写锁（写者优先），通过测试。

为批量图像处理添加原子进度条。

所有代码编译运行无误，Git 提交。

📖 学习心得
原子操作是实现无锁数据结构的基础，但正确使用内存顺序需要深刻理解。

无锁算法可以显著提高性能，但设计复杂，容易出现微妙的 bug（如 ABA 问题）。

进度条使用原子计数器简单高效，适合长时间运行的任务。

读写锁在多读少写场景下比互斥锁有更好的性能表现。
