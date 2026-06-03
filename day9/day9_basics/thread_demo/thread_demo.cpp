#include <iostream>
#include <thread>

void hello() {
    std::cout << "Hello from thread. thread id : " << std::this_thread::get_id() << std::endl;
}

int main() {
    std::thread t(hello);	//创建线程
    t.join();
     std::cout << "Main thread id: " << std::this_thread::get_id() << std::endl;
    return 0;
}
