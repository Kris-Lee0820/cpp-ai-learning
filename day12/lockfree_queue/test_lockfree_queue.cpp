#include "lockfree_queue.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    LockFreeQueue<int> queue(1024);
    std::atomic<bool> stop{false};
    int sum = 0;

    // 生产者线程
    std::thread producer([&]() {
	for (int i = 0; i < 100000; ++i) {
	    while (!queue.push(i)) {
		std::this_thread::yield();
	    }
	}	
	stop = true;
    });

    // 消费者线程
    std::thread consumer([&]() {
	int value;
	while (!stop || !queue.pop(value)) {
	    if (queue.pop(value)) {
		sum += value;
	    } else {
		std::this_thread::yield();
	    }
	}	
    });

    producer.join();
    consumer.join();
    std::cout << "Sum = " << sum << " (expected 4999950000)" << std::endl;
    return 0;
}
