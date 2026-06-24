#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>

// 计算斐伯纳契数的函数 （模拟耗时任务）
long long fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

class TaskQueue {
public:
    using Task = std::packaged_task<long long()>;    // 定义任务类型

    // 添加任务到队列
    std::future<long long> addTask(int n) {
        Task task(std::bind(fibonacci, n));
        auto future = task.get_future();
        {
            std::lock_guard<std::mutex> lock(mtx_);
            tasks_.push(std::move(task));
        }
        cond_.notify_one();
        return future;
    }

    // 获取任务 （供工作线程使用）
    bool getTask(Task& task) {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
        if (stop_ && tasks_.empty()) return false;
        task = std::move(tasks_.front());
        tasks_.pop();
        return true;
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mtx_);
        stop_ = true;
        cond_.notify_all();
    }

private:
    std::queue<Task> tasks_;
    std::mutex mtx_;
    std::condition_variable cond_;
    bool stop_ = false;
};

// 线程池类
class Threadpool {
public:
    explicit Threadpool(size_t numThreads) : taskQueue_() {
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] { workFunc(); });
        }
    }

    ~Threadpool() {
        taskQueue_.stop();
        for (auto& worker : workers_) {
            if (worker.joinable()) worker.join();
        }
    }

    // 提交任务并返回 future
    std::future<long long> submit(int n) {
        return taskQueue_.addTask(n);
    }

private:
    void workFunc() {
        TaskQueue::Task task([]() { return 0LL; });
        while (taskQueue_.getTask(task)) {
            task();    // 执行任务
        }
    }

    std::vector<std::thread> workers_;
    TaskQueue taskQueue_;
};

int main() {
    // 创建线程池
    Threadpool pool(std::thread::hardware_concurrency());

    std::cout << "提交 10 个斐波那契计算任务..." << std::endl;

    std::vector<std::future<long long>> results;
    for (int i = 30; i < 40; ++i) {
        results.push_back(pool.submit(i));
    }

    std::cout << "所有任务已提交，等待结果..." << std::endl;

    for (size_t i = 0; i < results.size(); ++i) {
        long long result = results[i].get();
        std::cout << "fibonacci(" << (30 + i) << ") = " << result << std::endl;
    }

    std::cout << "所有任务完成！" << std::endl;
    return 0;
}