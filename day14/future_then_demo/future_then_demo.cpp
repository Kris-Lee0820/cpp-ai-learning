#include <iostream>
#include <future>
#include <functional>
#include <thread>
#include <chrono>

// then 模板函数：链式调用异步任务
template<typename T, typename Func>
auto then(std::future<T>& future, Func&& func) -> std::future<decltype(func(std::declval<T>()))> {
    using ReturnType = decltype(func(std::declval<T>()));
    auto promise = std::make_shared<std::promise<ReturnType>>();
    auto nextFuture = promise->get_future();

    std::thread([promise, &future, func]() {
        try {
            auto result = future.get();
            promise->set_value(func(result));
        } catch (...) {
            promise->set_exception(std::current_exception());
        }
    }).detach();

    return nextFuture;
}

// 图像处理模拟函数
int readImage(const std::string& path) {
    std::cout << "读取图像: " << path << " (子线程 " << std::this_thread::get_id() << ")" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return 100;  // 模拟返回图像数据大小
}

int grayscale(int img_data) {
    std::cout << "灰度化处理 (子线程 " << std::this_thread::get_id() << ")" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    return img_data * 2;
}

int resizeImg(int img_data) {
    std::cout << "缩放图像 (子线程 " << std::this_thread::get_id() << ")" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return img_data / 2;
}

void saveImage(int img_data) {
    std::cout << "保存图像，最终数据大小: " << img_data << " (子线程 " << std::this_thread::get_id() << ")" << std::endl;
}

int main() {
    std::cout << "主线程 ID: " << std::this_thread::get_id() << std::endl;
    std::cout << "=== 异步任务链：读取 → 灰度化 → 缩放 → 保存 ===" << std::endl;

    // 启动第一个异步任务
    std::future<int> f1 = std::async(std::launch::async, readImage, "test.jpg");

    // 链式调用
    auto f2 = then(f1, [](int data) { return grayscale(data); });
    auto f3 = then(f2, [](int data) { return resizeImg(data); });

    // 最后一个任务：保存（无返回值）
    auto f4 = then(f3, [](int data) {
        saveImage(data);
        return data;
    });

    // 等待最终结果
    int final_result = f4.get();
    std::cout << "=== 处理完成，最终结果: " << final_result << " ===" << std::endl;

    return 0;
}