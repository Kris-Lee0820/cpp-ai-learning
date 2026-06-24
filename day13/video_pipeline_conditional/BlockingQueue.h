#ifndef BLOCKING_QUEUE_HPP
#define BLOCKING_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class BlockingQueue {
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable not_empty;
    std::condition_variable not_full;
    size_t capacity;

public:
    explicit BlockingQueue(size_t cap) : capacity(cap) {}

    void push(const T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        not_full.wait(lock, [this] { return queue.size() < capacity; });
        queue.push(item);
        lock.unlock();
        not_empty.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mtx);
        not_empty.wait(lock, [this] { return !queue.empty(); });
        T item = queue.front();
        queue.pop();
        lock.unlock();
        not_full.notify_one();
        return item;
    }

    bool try_push(const T& item) {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.size() >= capacity) return false;
        queue.push(item);
        not_empty.notify_one();
        return true;
    }

    bool try_pop(T& item) {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.empty()) return false;
        item = queue.front();
        queue.pop();
        not_full.notify_one();
        return true;
    }
};

#endif  // BLOCKING_QUEUE_HPP