Day 11 完整学习笔记：并行图像处理与性能优化
📌 今日学习内容概览
使用 std::async 并行处理多张图像（任务并行）。

实现自定义线程池并进行分块图像处理（数据并行）。

使用 Intel TBB 库实现并行高斯模糊，并与串行版本对比性能。

扩展挑战：

异步视频处理管道（生产者-消费者模型）。
支持优先级的线程池。
TBB 分块高斯模糊的完整实现与性能测试。
📚 第一部分：理论回顾
并行策略
策略	描述	适用场景
任务并行	不同独立任务同时执行	批量图像处理、独立文件处理
数据并行	将数据分块，每块执行相同操作	大图像滤波、矩阵运算
流水线并行	将任务分解为阶段，阶段间并行	视频处理（读-处理-写）
C++ 并行工具
std::async / std::future：高抽象，适合少量任务。

线程池：复用线程，适合大量小任务。

Intel TBB：高级任务调度，自动负载均衡，适合数据并行。

💻 第二部分：实践任务实现
任务一：std::async 并行批量图像处理
代码 parallel_batch_async.cpp

读取文件夹内所有 .jpg 文件。

对每张图片使用 std::async 异步执行 CLAHE 增强。

等待所有任务完成，统计耗时。

运行示例：

bash
./parallel_batch_async ./input_images ./output_async
加速比：4核机器上相比串行版本约 3.5 倍。

任务二：自定义线程池分块高斯模糊
线程池实现 ThreadPool.hpp

使用 std::queue 存储任务，std::thread 数组处理。

enqueue 返回 std::future 获取结果。

主程序 parallel_block_threadpool.cpp

将图像划分为 block_rows × block_cols 块。

每个块提交到线程池，并行执行高斯模糊。

合并结果，输出图像。

性能对比：

图像大小	串行耗时	线程池（16块）耗时	加速比
1920x1080	0.12s	0.04s	3.0x
4096x4096	0.85s	0.21s	4.0x
任务三：TBB 分块高斯模糊（含边界重叠处理）
关键点：为避免块边缘出现接缝，每个块需扩展重叠区域（半径为核大小/2），模糊后再裁剪回原始尺寸。

代码 tbb_block_blur.cpp

使用 tbb::parallel_for 和 tbb::blocked_range2d。

实现 blurBlockWithOverlap 处理重叠。

对比串行版本，输出加速比和最大差异。

运行结果（4096x4096，核15）：

text
Serial GaussianBlur time: 0.823 s
TBB block parallel time: 0.195 s
Speedup: 4.22
Max difference between serial and TBB result: 0
🧪 第三部分：扩展挑战实现
扩展挑战 1：异步视频处理管道（生产者-消费者）
代码 async_video_pipeline.cpp

三个异步任务：读取帧、处理帧（CLAHE）、显示帧。

使用 std::queue 和条件变量同步。

限制队列大小防止内存爆炸。

效果：处理速度提高约 2 倍（读 + 处理并行）。

扩展挑战 2：支持优先级的线程池
代码 PriorityThreadPool.hpp

使用 std::priority_queue 存储任务（<priority, task>）。

enqueue 增加优先级参数（默认 0，值越小优先级越高）。

利用 std::greater 使优先队列为小顶堆。

测试：提交不同优先级的任务，高优先级任务先执行。

cpp
pool.enqueue(0, [](){ std::cout << "Urgent\n"; });
pool.enqueue(10, [](){ std::cout << "Background\n"; });
扩展挑战 3：TBB 流水线并行处理视频
代码 tbb_pipeline_video.cpp

使用 tbb::parallel_pipeline 构建三阶段：读取（串行）→ 处理（并行）→ 显示（串行）。

自动负载均衡，提高吞吐量。

运行：处理视频帧时 FPS 提升约 1.5 倍（受限于显示速度）。

📊 性能总结
方法	适用场景	优点	缺点
std::async	少量独立任务	简单，无需管理线程	创建销毁开销大
线程池	大量相似任务	重用线程，可控	需要手动分块
TBB parallel_for	数据并行	自动负载均衡，边界友好	依赖第三方库
TBB parallel_pipeline	流水线任务	提高吞吐量	复杂度较高
❌ 遇到的问题与解决方案
问题	原因	解决方案
分块高斯模糊出现接缝	未处理边界重叠	扩展 ROI 包含重叠区域，再裁剪
线程池死锁	任务等待自己完成或循环等待	使用 std::future 和正确同步
TBB 编译链接错误	未安装 TBB 或未链接 -ltbb	sudo apt install libtbb-dev，编译时添加 -ltbb
视频管道内存爆炸	队列无上限	设置最大队列长度，条件变量限流
✅ 完成清单
并行批量图像处理（std::async）

自定义线程池分块高斯模糊

TBB 分块高斯模糊（含重叠处理，性能对比）

异步视频处理管道（生产者-消费者）

支持优先级的线程池

TBB 流水线并行处理视频（可选）

性能数据记录与笔记撰写
