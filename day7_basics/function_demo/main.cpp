#include <iostream>
#include <functional>

void hello() { std::cout << "Hello\n"; }

int main() {
    std::function<void()> f;
    f = hello;		//存储普通函数
    f();

    f = []() { std::cout << "Lambda\n"; };
    f();

    // 存储带参数的lambda
    std::function<int(int, int)> add = [](int a, int b) { return a + b; };
    std::cout << " 3 + 4 = " << add(3, 4) << std::endl;

    return 0;
}
