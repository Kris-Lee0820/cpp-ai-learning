#include "video_processor_threadPool.h"

// 处理单个条带的函数
void blurRegion(cv::Mat& output, const cv::Mat& input, int start_row, int end_row, int ksize) {
    cv::Rect roi(0, start_row, input.cols, end_row - start_row);
    cv::GaussianBlur(input(roi), output(roi), cv::Size(ksize, ksize), 0);
}

// 并行高斯模糊
void parallelBlur(const cv::Mat& src, cv::Mat& dst, int ksize, int numThreads) {
    dst = src.clone();
    ThreadPool pool(numThreads);
    std::vector<std::future<void>> futures;
    int rows = src.rows;
    int rowsPerThread = rows / numThreads;
    for (int i = 0; i < numThreads; ++i) {
	int start = i * rowsPerThread;
	int end = (i == numThreads - 1) ? rows : (i + 1) * rowsPerThread;
	futures.push_back(pool.enqueue([&dst, &src, start, end, ksize] {
            blurRegion(dst, src, start, end, ksize);
	}));
    }
    // for (auto& fut : futures) fut.get();
}

int main() {
    cv::Mat img = cv::imread("test.jpg");
    if (img.empty()) return -1;
    cv::Mat result;
    parallelBlur(img, result, 15, std::thread::hardware_concurrency());
    cv::imshow("Original", img);
    cv::imshow("Parallel Blur", result);
    cv::waitKey(0);
    return 0;
}
