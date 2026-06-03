#include <iostream>
#include <future>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

// 辅助函数：获取当前时间的字符串表示
std::string now_str() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    return std::ctime(&time_t);
}

// 耗时任务
int heavy_task(int id) {
    std::cout << "Task " << id << " started at " << now_str()
	      << " on thread " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(1s);	//模拟工作
    std::cout << "Task " << id << " finished at " << now_str();
    return id * id;
}

int main() {
    std::cout << "Main thread id : " << std::this_thread::get_id() << std::endl;
    std::cout << "Start time : " << now_str() << std::endl;

    // 1. async 策略：立即在新线程执行
    auto async_future = std::async(std::launch::async, heavy_task, 1);

    // 2. deferred 策略：不会立即执行，等待 get() 调用
    auto deferred_future = std::async(std::launch::deferred, heavy_task, 2);

    // 3. 默认策略（实现定义，通常为async | deferred）
    auto default_future = std::async(heavy_task, 3);

    // 主线程等待一段时间，观察哪些任务已经启动
    std::cout << "Main sleeping for 2 seconds..." << std::endl;
    std::this_thread::sleep_for(2s);

    if (deferred_future.wait_for(0s) == std::future_status::deferred) {
	std::cout << "Deferred task (2) is not yet started (lazy)." << std::endl;
    }

    std::cout << "Now calling get() on deferred task (2)..." << std::endl;
    int result2 = deferred_future.get();  // 此时任务才执行
    std::cout << "Result of deferred task: " << result2 << std::endl;
    
    // 获取其他任务的结果
    int result1 = async_future.get();
    int result3 = default_future.get();

    std::cout << "Final results: async=" << result1
              << ", deferred=" << result2
              << ", default=" << result3 << std::endl;
    std::cout << "End time: " << now_str();
    return 0;
}
