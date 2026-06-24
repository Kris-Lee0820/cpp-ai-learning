#include <iostream>
#include <future>
#include <chrono>
#include <thread>

// 模拟耗时计算任务
int compute_value(int input) {
    std::cout << "子线程 ID: " << std::this_thread::get_id() << " 开始计算..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    return input * input;
}

int main() {
    std::cout << "主线程 ID: " << std::this_thread::get_id() << std::endl;

    // 方式1：使用 async 启动异步任务
    std::future<int> result_future = std::async(std::launch::async, compute_value, 10);

    std::cout << "主线程可以执行其他操作..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::future_status status = result_future.wait_for(std::chrono::milliseconds(100));
    if (status == std::future_status::ready) {
        std::cout << "任务已完成！" << std::endl;
    } else if (status == std::future_status::timeout) {
        std::cout << "任务仍在执行中..." << std::endl;
    } else if (status == std::future_status::deferred) {
        std::cout << "任务延迟执行..." << std::endl;
    }

    // 阻塞等待并获取结果
    int result = result_future.get();
    std::cout << "计算结果：" << result << std::endl;
    return 0;
}