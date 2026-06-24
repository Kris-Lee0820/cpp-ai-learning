#include <iostream>
#include <future>
#include <thread>
#include <chrono>

// 普通加法函数
int add_num(int a, int b) {
    std::cout << "子线程ID :" << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return a + b;
}

int main() {
    std::cout << "主线程ID :" << std::this_thread::get_id() << std::endl;

    // 1.封装任务：将函数打包成 packaged_task
    std::packaged_task<int(int, int)> task(add_num);

    // 2.获取与任务绑定的 future
    std::future<int> result = task.get_future();

    // 3.交给子线程执行（必须使用 std::move 转移任务所有权)
    std::thread work_thread(std::move(task), 10, 20);

    // 主线程可以做其他事情...
    std::cout << "主线程继续执行..." << std::endl;

    // 4.等待线程完成
    work_thread.join();

    // 5.获取执行结果
    int res = result.get();
    std::cout << "10 + 20 = " << res << std::endl;
    
    return 0;
}