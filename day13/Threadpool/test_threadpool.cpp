#include "Threadpool.h"
#include <iostream>
#include <chrono>

int main() {
    // 创建线程池（使用硬件并发数）
    Threadpool pool;

    // 提交一批任务
    std::vector<std::future<int>> results;
    for (int i = 0; i < 20; ++i) {
        results.push_back(pool.enqueue([i] {
            // 模拟耗时操作
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return i * i;
        }));
    }

    // 获取结果
    for (size_t i = 0; i < results.size(); ++i) {
        int value = results[i].get();
        std::cout << "Task " << i << " result = " << value << std::endl;
    }

    std::cout << "All tasks completed." << std::endl;
    return 0;
}