#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <future>
#include <thread>

class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

    void workerLoop() {
	while (true) {
	    std::function<void()> task;
	    {
		std::unique_lock<std::mutex> lock(queue_mutex);
		condition.wait(lock, [this] { return stop || !tasks.empty(); });
		if (stop && tasks.empty()) return;
		task = std::move(tasks.front());
		tasks.pop();
	    }
	    task();
	}
    }

public:
    ThreadPool(size_t nums_thread) : stop(false) {
	for (size_t i = 0; i < nums_thread; ++i) {
	    workers.emplace_back([this] { workerLoop(); });
	}
    }

    ~ThreadPool() {
	{
	    std::unique_lock<std::mutex> lock(queue_mutex);
	    stop = true;
	}
	condition.notify_all();
	for (std::thread& worker : workers) worker.join();
    }

    template<typename F>
    auto enqueue(F&& f) -> std::future<decltype(f())> {
	using return_type = decltype(f());
	auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
	std::future<return_type> res = task->get_future();
	{
	    std::unique_lock<std::mutex> lock(queue_mutex);
	    if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
	    tasks.emplace([task]() { (*task)(); });
	}
	condition.notify_one();
	return res;
    }
};
