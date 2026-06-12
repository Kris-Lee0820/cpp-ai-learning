#include <opencv2/opencv.hpp>
#include <future>
#include <vector>
#include <filesystem>
#include <chrono>

cv::Mat processImage(const cv::Mat& src) {
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
        std::cerr << "Usage: ./parallel_batch_async input_dir output_dir" << std::endl;
        return -1;
    }
    std::filesystem::path input_dir(argv[1]);
    std::filesystem::path output_dir(argv[2]);
    if (!std::filesystem::exists(output_dir)) std::filesystem::create_directories(output_dir);

    std::vector<std::filesystem::path> image_paths;
    for (const auto& entry : std::filesystem::directory_iterator(input_dir)) {
	if (entry.path().extension() == ".jpg" || entry.path().extension() == ".png")
	    image_paths.push_back(entry.path());
    }

    // 异步任务列表
    std::vector<std::future<void>> futures;

    auto start = std::chrono::steady_clock::now();

    for (const auto& path : image_paths) {
	futures.push_back(std::async(std::launch::async, [path, &output_dir]() {
	    cv::Mat img = cv::imread(path.string());
	    if (img.empty()) return;
	    cv::Mat processed = processImage(img);
	    cv::imwrite((output_dir / path.filename()).string(), processed);
	}));
    }

    for (auto& fut : futures) {
	try {
	    fut.get();
	} catch (const std::exception& e) {
	    std::cerr << "异步任务失败：" << e.what() << std::endl;
	}
    }

    auto end = std::chrono::steady_clock::now();
    std::cout << "Processed " << image_paths.size() << " images in " << std::chrono::duration<double>(end - start).count() << " seconds" << std::endl;

    return 0;
}
