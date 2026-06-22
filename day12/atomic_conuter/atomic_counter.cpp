#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <mutex>

const int NUM_THREADS = 8;
const long long ITERATIONS = 100'000'000;

// 使用原子变量
void atomicCounter(std::atomic<long long>& counter) {
    for (long long i = 0; i < ITERATIONS; ++i) {
	counter.fetch_add(1, std::memory_order_relaxed);
    }
}

void mutexCounter(long long& counter, std::mutex& mtx) {
    for (long long i = 0; i < ITERATIONS; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        ++counter;
    }
}

int main() {
    // 测试原子变量
    std::atomic<long long> atomic_cnt(0);
    auto start = std::chrono::steady_clock::now();
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
	threads.emplace_back(atomicCounter, std::ref(atomic_cnt));
    }
    for (auto& t : threads) t.join();
    auto end = std::chrono::steady_clock::now();
    std::cout << "Atomic counter: " << atomic_cnt.load()
              << " time: " << std::chrono::duration<double>(end - start).count() << "s\n";

    // 测试互斥锁
    long long mutex_cnt = 0;
    std::mutex mtx;
    threads.clear();
    start = std::chrono::steady_clock::now();
    for (int i = 0; i < NUM_THREADS; ++i) {
	threads.emplace_back(mutexCounter, std::ref(mutex_cnt), std::ref(mtx));
    }
    for (auto& t : threads) t.join();
    end = std::chrono::steady_clock::now();
    std::cout << "Mutex counter: " << mutex_cnt
              << " time: " << std::chrono::duration<double>(end - start).count() << "s\n";

    return 0;
}
