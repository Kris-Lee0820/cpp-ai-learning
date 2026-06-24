#include <iostream>
#include <future>
#include <thread>
#include <chrono>

// 工作线程：通过 promise 向主线程传递结果
void work_task(std::promise<int>& p, int input) {
    std::cout << "子线程 ID: " << std::this_thread::get_id() << " 开始工作..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    int result = input * 3 + 10;
    p.set_value(result);    // 设置值，唤醒等待的 future
    std::cout << "子线程： 已设置值 " << result << std::endl;
}

int main() {
    std::cout << "主线程 ID :" << std::this_thread::get_id() << std::endl;

    // 创建 promise 和 future
    std::promise<int> p;
    std::future<int> fu = p.get_future();

    // 启动子线程，传递 promise
    std::thread t(work_task, std::ref(p), 5);

    std::cout << "主线程：等待子线程计算结果..." << std::endl;

    // 阻塞等待结果
    int result = fu.get();
    std::cout << "主线程：收到结果 = " << result << std::endl;

    t.join();
    return 0;
}