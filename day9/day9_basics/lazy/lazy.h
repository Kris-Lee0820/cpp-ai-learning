#include <iostream>
#include <future>
#include <functional>
#include <utility>
#include <optional>

template<typename T>
class Lazy {
public:
    // 构造函数：接受一个可调用对象（函数、lambda等）
    template<typename F>
    Lazy(F&& func) : m_func(std::forward<F>(func)) {}

    // 显式求值：返回结果，若已求值则直接返回缓存
    T get() {
	if (!m_future.valid()) {
	    m_future = std::async(std::launch::deferred, m_func);
	}
	return m_future.get();
    }

    // 超时版本：尝试在给定时间内计算，若超时则返回默认值或抛出异常
    template<typename Rep, typename Period>
    T get_for(const std::chrono::duration<Rep, Period>& rel_time) {
	if (m_cache) return *m_cache;
	// 使用异步任务和 future 实现超时
	auto fut = std::async(std::launch::async, m_func);
	if (fut.wait_for(rel_time) == std::future_status::ready) {
	    m_cache = fut.get();
	    return *m_cache;
	} else {
            // 超时：可以选择抛出异常或返回默认值
	    throw std::runtime_error("Computation timeout");
	}
    }

    // 隐式转换：允许 Lazy<T> 当作 T 使用（自动求值）
    operator T() {
	return get();
    }

    // 重置求值状态（可选，用于重新计算）
    void reset() {
	m_future = std::future<T>();
    }

private:
    std::function<T()> m_func;
    std::future<T> m_future;
    mutable std::optional<T> m_cache; 
};

// 辅助函数：推导类型，简化创建
template<typename F>
auto make_lazy(F&& f) -> Lazy<decltype(f())> {
    return Lazy<decltype(f())>(std::forward<F>(f));
}
