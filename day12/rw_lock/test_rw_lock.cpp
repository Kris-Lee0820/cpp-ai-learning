#include "rw_lock.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

RWLock rwlock;
int shared_data = 0;

void reader(int id) {
    rwlock.read_lock();
    std::cout << "Reader " << id << " sees value: " << shared_data << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    rwlock.read_unlock();
}

void writer(int id, int value) {
    rwlock.write_lock();
    shared_data = value;
    std::cout << "Writer " << id << " sets value to: " << shared_data << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    rwlock.write_unlock();
}

int main() {
    std::vector<std::thread> threads;
    // 启动写者
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(writer, i, i * 100);
    }
    // 启动读者
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(reader, i);
    }
    for (auto& t : threads) t.join();
    return 0;
}
