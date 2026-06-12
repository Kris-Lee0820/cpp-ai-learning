#include <opencv2/opencv.hpp>
#include"Threadpool.h"
#include <chrono>
#include <cmath>

cv::Mat blurBlock(const cv::Mat& block, int kernelSize) {
    cv::Mat blurred;
    cv::GaussianBlur(block, blurred, cv::Size(kernelSize, kernelSize), 0);
    return blurred;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./parallel_block_threadpool <image_path>" << std::endl;
        return -1;
    }

    cv::Mat img = cv::imread(argv[1]);
    if (img.empty()) return -1;

    int num_threads = std::thread::hardware_concurrency();
    ThreadPool pool(num_threads);
    int block_rows = 4;   // 垂直切分成4块
    int block_cols = 4;   // 水平切分成4块，共16块
    int kernel = 15;

    int block_h = img.rows / block_rows;
    int block_w = img.cols / block_cols;

    std::vector<std::future<cv::Mat>> futures;
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < block_rows; ++i) {
	for (int j = 0; j < block_cols; ++j) {
	    cv::Rect roi(j * block_w, i * block_h, block_w, block_h);
	    // 注意边界处理，最后一块可能略大，因为将剩余部分都放到了最后一块
	    if (i == block_rows - 1) roi.height = img.rows - i * block_h;
	    if (j == block_cols - 1) roi.width = img.cols - j * block_w;
	    cv::Mat block = img(roi).clone();
	    futures.push_back(pool.enqueue([block, kernel]() {
		return blurBlock(block, kernel);			    
	    }));
	}
    }

    cv::Mat result(img.size(), img.type());
    int idx = 0;
    for (int i = 0; i < block_rows; ++i) {
	for (int j = 0; j < block_cols; ++j) {
	    cv::Mat blurred = futures[idx++].get();
	    cv::Rect roi(j * block_w, i * block_h, blurred.cols, blurred.rows);
	    blurred.copyTo(result(roi));
	}
    }

    auto end = std::chrono::steady_clock::now();
    std::cout << "Block processing with " << num_threads << " threads took "
              << std::chrono::duration<double>(end - start).count() << " seconds" << std::endl;

    cv::imwrite("result_threadpool.jpg", result);
    cv::imshow("Result", result);
    cv::waitKey(0);
    return 0;
}
