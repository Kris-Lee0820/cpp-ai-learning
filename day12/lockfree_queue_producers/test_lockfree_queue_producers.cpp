#include "lockfree_queue_producers.h"
#include <atomic>
#include <thread>
#include <iostream>

int main() {
    LockFreeQueueProducers<int> queue(1024);
    std::atomic<bool> stop{false};
    int sum = 0;

    std::thread producer([&]() {
	for (int i = 0; i < 100000; ++i) {
	    while (!queue.push(i)) {
		std::this_thread::yield();
	    }
	}	
	stop = true;
    });

    std::thread consumer([&]() {
	int value;
	while (true) {
	    if (queue.pop(value)) {
		sum += value;
	    } else if (stop) {
	   	break;
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
