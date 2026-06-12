#ifndef PRIORITY_THREAD_POOL_HPP
#define PRIORITY_THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <utility>

class PriorityThreadPool {
public:
    explicit PriorityThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) return;
                        // 取出优先级最高的任务（最小 first）
                        task = std::move(tasks.top().second);
                        tasks.pop();
                    }
                    task();
                }
            });
    }

    // 提交任务，指定优先级（默认 0，数值越小优先级越高）
    template<class F, class... Args>
    auto enqueue(int priority, F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> 
    {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop) throw std::runtime_error("enqueue on stopped PriorityThreadPool");
            // 将任务包装为 void() 函数，并和优先级一起存入优先队列
            tasks.emplace(priority, [task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    ~PriorityThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers) worker.join();
    }

private:
    struct Compare {
        bool operator()(const std::pair<int, std::function<void()>> &a,
                      const std::pair<int, std::function<void()>> &b) const {
            return a.first > b.first;
        }
    };

    std::vector<std::thread> workers;
    // 优先队列：first = 优先级（小顶堆），second = 任务函数
    std::priority_queue<
        std::pair<int, std::function<void()>>,
        std::vector<std::pair<int, std::function<void()>>>,
        Compare
    > tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

#endif // PRIORITY_THREAD_POOL_HPP
