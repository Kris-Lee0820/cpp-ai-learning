#include <opencv2/opencv.hpp>
#include <future>
#include <vector>
#include <thread>
#include <filesystem>
#include <chrono>
#include <atomic>
#include <iostream>
#include <iomanip>

namespace fs = std::filesystem;

cv::Mat processImage(const cv::Mat& src) {
    // 模拟耗时操作：CLAHE
    cv::Mat lab, result;
    cv::cvtColor(src, lab, cv::COLOR_BGR2Lab);
    std::vector<cv::Mat> channels;
    cv::split(lab, channels);
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(channels[0], channels[0]);
    cv::merge(channels, lab);
    cv::cvtColor(lab, result, cv::COLOR_Lab2BGR);
    return result;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: ./parallel_batch_with_progress input_dir output_dir" << std::endl;
        return -1;
    }
    fs::path input_dir(argv[1]);
    fs::path output_dir(argv[2]);
    if (!fs::exists(output_dir)) fs::create_directories(output_dir);

    std::vector<fs::path> image_paths;
    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (entry.path().extension() == ".jpg" || entry.path().extension() == ".png") {
            image_paths.push_back(entry.path());
        }
    }

    size_t total = image_paths.size();
    std::atomic<size_t> completed{0};

    std::vector<std::future<void>> futures;
    auto start = std::chrono::steady_clock::now();

    // 启动进度监控线程
    std::atomic<bool> stop_monitor{false};
    std::thread monitor([&]() {
        while (!stop_monitor) {
            size_t done = completed.load(std::memory_order_acquire);
            double percent = 100.0 * done / total;
            std::cout << "\rProgress: " << std::fixed << std::setprecision(1) << percent << "% (" 
                      << done << "/" << total << ")" << std::flush;
            if (done == total) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    // 提交异步任务
    for (const auto& path : image_paths) {
        futures.push_back(std::async(std::launch::async, [path, &output_dir, &completed]() {
            cv::Mat img = cv::imread(path.string());
            if (img.empty()) return;
            cv::Mat processed = processImage(img);
            cv::imwrite((output_dir / path.filename()).string(), processed);
            completed.fetch_add(1, std::memory_order_release);
        }));
    }

    for (auto& fut : futures) fut.wait();
    stop_monitor = true;
    monitor.join();

    auto end = std::chrono::steady_clock::now();
    std::cout << "\nAll images processed in " 
              << std::chrono::duration<double>(end - start).count() << " seconds." << std::endl;
    
    return 0;
} 