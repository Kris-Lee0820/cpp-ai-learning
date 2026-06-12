#include "PriorityThreadpool.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    PriorityThreadPool pool(4);  // 4 个工作线程

    std::vector<std::future<int>> results;

    // 提交不同优先级的任务（数值越小越先执行）
    results.push_back(pool.enqueue(10, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Task 10 (low priority) done" << std::endl;
        return 10;
    }));

    results.push_back(pool.enqueue(0, []() {
        std::cout << "Task 0 (highest priority) done" << std::endl;
        return 0;
    }));

    results.push_back(pool.enqueue(5, []() {
        std::cout << "Task 5 (medium priority) done" << std::endl;
        return 5;
    }));

    results.push_back(pool.enqueue(1, []() {
        std::cout << "Task 1 (high priority) done" << std::endl;
        return 1;
    }));

    // 等待所有任务完成并获取结果
    for (auto& fut : results) {
        std::cout << "Result: " << fut.get() << std::endl;
    }

    return 0;
}
