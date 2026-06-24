#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <chrono>

// 生产者：计算一个值
int produce_value() {
    std::cout << "生产者: 开始计算 (线程 " << std::this_thread::get_id() << ")" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    int result = 42;
    std::cout << "生产者: 计算结果 = " << result << std::endl;
    return result;
}

// 消费者1：打印结果
void consumer1(int value) {
    std::cout << "消费者1: 收到值 = " << value << " (线程 " << std::this_thread::get_id() << ")" << std::endl;
}

// 消费者2：对结果进行平方
void consumer2(int value) {
    int squared = value * value;
    std::cout << "消费者2: 收到值 = " << value << ", 平方 = " << squared
              << " (线程 " << std::this_thread::get_id() << ")" << std::endl;
}

// 消费者3：判断奇偶
void consumer3(int value) {
    std::cout << "消费者3: 收到值 = " << value << ", 是 " << (value % 2 == 0 ? "偶数" : "奇数")
              << " (线程 " << std::this_thread::get_id() << ")" << std::endl;
}

int main() {
    std::cout << "=== shared_future 广播演示 ===" << std::endl;
    std::cout << "主线程 ID: " << std::this_thread::get_id() << std::endl;

    // 创建普通 future
    std::future<int> future = std::async(std::launch::async, produce_value);

    // 转换为 shared_future（可多次获取）
    std::shared_future shared_future = future.share();

    // 启动多个消费者线程，都等待同一个 shared_future
    std::vector<std::thread> consumers;
    consumers.emplace_back([shared_future]() { consumer1(shared_future.get()); });
    consumers.emplace_back([shared_future]() { consumer2(shared_future.get()); });
    consumers.emplace_back([shared_future]() { consumer3(shared_future.get()); });

    // 主线程也可以多次获取
    int main_value = shared_future.get();
    std::cout << "主线程: 也收到了值 = " << main_value << std::endl;

    for (auto& t : consumers) {
        if (t.joinable()) t.join();
    }

    return 0;
}