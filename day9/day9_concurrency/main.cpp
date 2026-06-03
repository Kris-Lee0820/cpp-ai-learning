#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <future>

// 全局互斥量 (用于保护控制台输出，避免乱序)
std::mutex cout_mutex;

// 线程执行的函数：对图像的一部分进行高斯模糊
void blurRegion(cv::Mat& img, int startRow, int endRow, int kernelSize) {
    {
	std::lock_guard<std::mutex> lock(cout_mutex);
	std::cout << "Thread " << std::this_thread::get_id() 
		  << " processing rows [" << startRow << ", " << endRow << "]" << std::endl;
    }
    // 注意：OpenCV 的高斯模糊不支持部分图像，但我们可以提取 ROI，然后模糊并复制回去
    cv::Rect roi(0, startRow, img.cols, endRow - startRow);
    cv::Mat region = img(roi);
    cv::GaussianBlur(region, region, cv::Size(kernelSize, kernelSize), 0);
}

void parallelGaussianBlur(const cv::Mat& src, cv::Mat& dst, int kernel_size, int num_segments = 4) {
    if (src.empty()) return;
    dst = src.clone();	 // 先复制原图，然后修改副本的各个块
    
    std::vector<std::future<void>> futures;
    int rows_per_segment = dst.rows / num_segments;
    for (int i = 0; i < num_segments; ++i) {
	int start_row = i * rows_per_segment;
	int end_row = (i == num_segments - 1) ? src.rows : (i + 1) * rows_per_segment;
	// 启动异步任务，注意传递引用：std::ref(dst) 和 std::ref(src)
	auto fut = std::async(std::launch::async, blurRegion, std::ref(dst), start_row, end_row, kernel_size);
	futures.push_back(std::move(fut));
    }

    // // 等待所有任务完成（调用 wait() 而不是 get()）
    for (const auto& fut : futures) {
	if (fut.valid()) {
	    fut.wait();	// 对于 std::future<void>，wait() 会阻塞直到任务完成
	}
    }
}

int main(int argc, char** argv) {
    cv::Mat frame;
    if (argc > 1) {
	frame = cv::imread(argv[1]);
    } else {
	// 如果没有提供图片，生成一个测试图像
	frame = cv::Mat(480, 640, CV_8UC3, cv::Scalar(100, 150, 200));
	cv::putText(frame, "Test Image", cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
    }
    if (frame.empty()) {
	std::cerr << "Error: cannot load image" << std::endl;
	return -1;
    }

    int num_threads = std::thread::hardware_concurrency();	// 获取CPU核心数
    if (num_threads == 0) num_threads = 4;
    std::cout << "Detected " << num_threads << " CPU cores. Using " << num_threads << " threads." << std::endl;

    int kernelSize = 15;	// 滤波核大小
    int rows = frame.rows;
    int rows_per_thread = rows / num_threads;
    int remainder = rows % num_threads;

    // ========== 单线程版本 ==========
    cv::Mat single_result = frame.clone();
    auto start_single = std::chrono::steady_clock::now();
    cv::GaussianBlur(single_result, single_result, cv::Size(kernelSize, kernelSize), 0);

    auto end_single = std::chrono::steady_clock::now();
    double single_time = std::chrono::duration<double>(end_single - start_single).count();
    std::cout << "Single thread time : " << single_time << " seconds" << std::endl;

    // ========== 多线程版本 ==========
    cv::Mat multi_result = frame.clone();
    std::vector<std::thread> threads;
    auto start_multi = std::chrono::steady_clock::now();

    int start_row = 0;
    for (int i = 0; i < num_threads; ++i) {
	int end_row = start_row + rows_per_thread + (i < remainder ? 1 : 0);
	// 注意：由于 blurRegion 直接修改 multi_result 的 ROI，我们需要确保每个线程操作不同的行范围，不会有重叠
	threads.emplace_back(blurRegion, std::ref(multi_result), start_row, end_row, kernelSize);
	start_row = end_row;
    }
    for (auto& t: threads) t.join();
    auto end_multi = std::chrono::steady_clock::now();
    double multi_time = std::chrono::duration<double>(end_multi - start_multi).count();
    std::cout << "Multi-thread time : " << multi_time << " second" << std::endl;
    
    // ========== 异步版本 ==========
    cv::Mat future_result;
    auto start_future = std::chrono::steady_clock::now();
    parallelGaussianBlur(frame, future_result, 15, 4);
    auto end_future = std::chrono::steady_clock::now();
    double future_time = std::chrono::duration<double>(end_future - start_future).count();
    std::cout << "future async time : " << future_time << " second" << std::endl;

    // 显示结果
    cv::imshow("Original", frame);
    cv::imshow("Single thread blurred", single_result);
    cv::imshow("Multi thread blurred", multi_result);
    cv::imshow("future async blurred", future_result);
    cv::waitKey(0);
    return 0;
}
