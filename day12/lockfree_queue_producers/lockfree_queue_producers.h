#ifndef LOCKFREE_QUEUE_PRODUCERS_H
#define LOCKFREE_QUEUE_PRODUCERS_H

#include <vector>
#include <atomic>

template<typename T>
class LockFreeQueueProducers {
private:
    std::vector<T> buffer;
    size_t capacity{0};
    std::atomic<size_t> head{0};
    std::atomic<size_t> tail{0};
public:
    explicit LockFreeQueueProducers(size_t size) : buffer(size), capacity(size) {}
    bool push(const T& item) {
	size_t t = tail.load(std::memory_order_relaxed);
	while (true) {
	    size_t next = (t + 1) % capacity;
	    if (next == head.load(std::memory_order_acquire))
	        return false;	// 满
	    if (tail.compare_exchange_weak(t, next, std::memory_order_release, std::memory_order_relaxed))
		break;
	}
	buffer[t] = item;
	return true;
    }

    bool pop(T& item) {
	size_t h = head.load(std::memory_order_relaxed);
	while (true) {
	    if (h == tail.load(std::memory_order_acquire))
		return false;	// 空
	    item = buffer[h];
            if (head.compare_exchange_weak(h, (h+1)%capacity, std::memory_order_release, std::memory_order_relaxed))
		break;
	}
	return true;
    }
};

#endif	// LOCKFREE_QUEUE_PRODUCERS_H
