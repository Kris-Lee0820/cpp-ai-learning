#include "BlockingQueue.h"
#include <iostream>
#include <thread>
#include <vector>

int main() {
    BlockingQueue<int> queue(5);
    std::vector<int> results;
    std::mutex res_mtx;

    // 生产者
    std::thread producer([&]() {
        for (int i = 0; i < 20; ++i) {
            queue.push(i);
            std::cout << "Produced: " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    });

    // 消费者
    std::thread consumer([&]() {
        for (int i = 0; i < 20; ++i) {
            int val = queue.pop();
            std::lock_guard<std::mutex> lock(res_mtx);
            results.push_back(val);
            std::cout << "Consumed: " << val << std::endl;
        }
    });

    producer.join();
    consumer.join();

    std::cout << "All values: ";
    for (int v : results) std::cout << v << " ";
    std::cout << std::endl;

    return 0;
}