#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;
int counter = 0;

void increment() {
    for (int i = 0; i < 100000; ++i) {
	std::lock_guard<std::mutex> lock(mtx);	//自动加锁解锁
	++counter;
    }
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
	threads.emplace_back(increment);
    }
    for (auto &t : threads) {
	t.join();
    }
    std::cout << "Counter: " << counter << std::endl;
    return 0;
}
