#include <chrono>
#include <thread>
#include "lazy.h"

int expensive_computation(int x) {
    std::cout << "Computing " << x << "^2...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return x * x;
}

int main() {
    auto lazy_square = make_lazy([]() { return expensive_computation(5); });

    std::cout << "Lazy object created, nothing computed yet.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "Now forcing evaluation...\n";
    int result = lazy_square.get();   // 这里才真正计算
    std::cout << "Result: " << result << "\n";

    // 再次获取，直接返回缓存值，不会重新计算
    std::cout << "Getting again (cached): " << lazy_square.get() << "\n";

    // 使用隐式转换
    int implicit = lazy_square;
    std::cout << "Implicit conversion: " << implicit << "\n";
    
    auto lazy = make_lazy([](){
	std::this_thread::sleep_for(std::chrono::seconds(2));
        return 100;	
    });

    try {
	int val = lazy.get_for(std::chrono::milliseconds(2500));
	std::cout << "get for val is : " << val << std::endl;
    } catch (...) {
 	std::cout << "Timeout\n";
    }
    return 0;
}
