#ifndef LOCKFREE_QUEUE_HPP
#define LOCKFREE_QUEUE_HPP

#include <atomic>
#include <vector>

template<typename T>
class LockFreeQueue {
private:
    std::vector<T> buffer;
    std::atomic<size_t> head{0};
    std::atomic<size_t> tail{0};
    size_t capacity;

public:
    explicit LockFreeQueue(size_t size) : buffer(size), capacity(size) {}

    bool push(const T& item) {
    size_t t = tail.load(std::memory_order_relaxed);
    while (true) {
        size_t next = (t + 1) % capacity;
        if (next == head.load(std::memory_order_acquire))
            return false; // 满
        if (tail.compare_exchange_weak(t, next, std::memory_order_release, std::memory_order_relaxed))
            break;
    }
    buffer[t] = item;
    return true;
}

    bool pop(T& item) {
	size_t h = head.load(std::memory_order_relaxed);
	if (h == tail.load(std::memory_order_acquire)) 
	    return false;
	item = buffer[h];
	head.store((h + 1) % capacity, std::memory_order_release);
	return true;
    }
};

#endif	// LOCKFREE_QUEUE_HPP
