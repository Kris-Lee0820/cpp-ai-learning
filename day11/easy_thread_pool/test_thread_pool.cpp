#include "easy_thread_pool.h"

int main() {
    ThreadPool pool(4);
    auto future = pool.enqueue([](int a, int b) { return a + b; }, 3, 4);
    std::cout << "Result : " << future.get() << std::endl;
    return 0;
}
