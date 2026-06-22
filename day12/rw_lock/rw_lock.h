#ifndef RW_LOCK_HPP
#define RW_LOCK_HPP

#include <atomic>
#include <thread>

class RWLock {
private:
    std::atomic<int> reader_count{0};
    std::atomic<int> writer_waiting{0};
    std::atomic<bool> writer_active{false};

public:
    void read_lock() {
		// 如果有写者正在写或等待写，则自旋等待
		while (writer_active.load(std::memory_order_acquire) || 
			writer_waiting.load(std::memory_order_acquire) > 0) {
			std::this_thread::yield();
		}
		reader_count.fetch_add(1, std::memory_order_release);
    }

    void read_unlock() {
		reader_count.fetch_sub(1, std::memory_order_release);
    }

    void write_lock() {
		// 增加等待写者计数
		writer_waiting.fetch_add(1, std::memory_order_release);
		// 等待当前读者全部退出且没有其他写者
		while (reader_count.load(std::memory_order_acquire) > 0 || 
			writer_active.load(std::memory_order_acquire)) {
			std::this_thread::yield();
		}
		// 尝试获取写锁
		bool expected = false;
		while (!writer_active.compare_exchange_weak(expected, true, std::memory_order_acq_rel, std::memory_order_acquire)) {
			expected = false;
			std::this_thread::yield();
		}
		writer_waiting.fetch_sub(1, std::memory_order_release);
    }

	void write_unlock() {
		writer_active.store(false, std::memory_order_release);
	}

};

#endif	// RW_LOCK_HPP
