#include <opencv2/opencv.hpp>
#include <future>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

// 阶段1：读取图像
cv::Mat readImage(const std::string& path) {
    std::cout << "[读取] " << path << " (线程 " << std::this_thread::get_id() << ")" << std::endl;
    cv::Mat img = cv::imread(path);
    if (img.empty()) {
        throw std::runtime_error("无法读取图像: " + path);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));    // 模拟I/O延迟
    return img;
}

// 阶段2：处理图像（CLAHE增强）
cv::Mat processImage(const cv::Mat& src) {
    std::cout << "[处理] 图像尺寸 " << src.cols << "x" << src.rows
              << " (线程 " << std::this_thread::get_id() << ")" << std::endl;
    
    cv::Mat lab, result;
    cv::cvtColor(src, lab, cv::COLOR_BGR2Lab);
    std::vector<cv::Mat> channels;
    cv::split(lab, channels);

    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(channels[0], channels[0]);

    cv::merge(channels, lab);
    cv::cvtColor(lab, result, cv::COLOR_Lab2BGR);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));  // 模拟计算延迟
    return result;
}

// 阶段3：保存图像
void saveImage(const cv::Mat& img, const std::string& path) {
    std::cout << "[保存] " << path << " (线程 " << std::this_thread::get_id() << ")" << std::endl;
    cv::imwrite(path, img);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));  // 模拟I/O延迟
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <输入图片路径> [输出路径]" << std::endl;
        return -1;
    }

    std::string input_path = argv[1];
    std::string output_path = (argc > 2) ? argv[2] : "output_pipeline.jpg";

    std::cout << "=== 异步图像处理流水线 ===" << std::endl;
    std::cout << "输入: " << input_path << std::endl;
    std::cout << "输出: " << output_path << std::endl;

    auto start = std::chrono::steady_clock::now();

    try
    {
        // 阶段1：异步读取
        auto future_read = std::async(std::launch::async, readImage, input_path);

        // 阶段2：读取完成后异步处理（使用 then 链式调用）
        auto future_process = std::async(std::launch::async, [&future_read]() {
            cv::Mat image = future_read.get();  // 等待读取完成
            return processImage(image);
        });

        // 阶段3：处理完成后异步保存
        auto future_save = std::async(std::launch::async, [&future_process, &output_path]() {
            cv::Mat processed = future_process.get();   // 等待处理完成
            saveImage(processed, output_path);
            return true;
        });

        // 等待保存完成
        bool success = future_save.get();

        auto end = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();

        if (success) {
            std::cout << "=== 处理完成！耗时 " << elapsed << " 秒 ===" << std::endl;
            std::cout << "输出文件: " << output_path << std::endl;
        }

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }
    
    return 0;
}