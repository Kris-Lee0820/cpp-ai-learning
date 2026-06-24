#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <utility>
#include <atomic>

class Threadpool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    mutable std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
public:
    // 构造函数：启动指定数量的工作线程
    explicit Threadpool(size_t threads = std::thread::hardware_concurrency()) : stop(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        // 等待条件：有任务或停止
                        condition.wait(lock, [this] {
                            return stop || !tasks.empty();
                        });
                        // 如果停止且任务队列为空，退出线程
                        if (stop && tasks.empty())
                            return;
                        // 取出任务
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    // 禁止拷贝和赋值构造
    Threadpool(const Threadpool&) = delete;
    Threadpool& operator=(const Threadpool&) = delete;

    // 提交任务：接受可调用对象和参数，返回 std::future
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        // 创建 packaged_task 封装任务
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            // 将任务包装为 void() 函数，存入队列
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return result;
    }

    // 析构函数：停止所有线程，等待它们结束
    ~Threadpool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (auto &worker : workers) {
            if (worker.joinable())
                worker.join();
        }
    }

    // 获取当前任务队列大小（调试用）
    size_t pending_tasks() const {
        std::lock_guard<std::mutex> lock(queue_mutex);
        return tasks.size();
    }
};

#endif  // THREAD_POOL_HPP