#include <opencv2/opencv.hpp>
#include <tbb/tbb.h>
#include <chrono>
#include <iostream>
#include <cmath>

// 计算处理块时需要的重叠区域大小（高斯核半径）
int getOverlapSize(int kernel_size) {
    return kernel_size / 2;	// 常见的高斯核半径，确保边缘不缺失
}

// 对图像的一个区域（包含重叠扩展区域）进行高斯模糊，然后裁剪回原始块
cv::Mat blurBlockWithOverlap(const cv::Mat& whole_img, const cv::Rect& roi, int kernel_size) {
    int overlap = getOverlapSize(kernel_size);
    // 扩展 ROI 以包含重叠区域
    // 左边界向左移动 overlap，但至少为 0；
    // 上边界向上移动 overlap，但至少为 0；
    // 右边界向右移动 overlap，但最多为图像宽度；
    // 下边界向下移动 overlap，但最多为图像高度。
    cv::Rect extended_roi(
	std::max(0, roi.x - overlap),
	std::max(0, roi.y - overlap),
	std::min(whole_img.cols - (roi.x - overlap), roi.width + 2 * overlap),
	std::min(whole_img.rows - (roi.y - overlap), roi.height + 2 * overlap)
    );
    cv::Mat extended = whole_img(extended_roi).clone();
    cv::Mat blurred;
    cv::GaussianBlur(extended, blurred, cv::Size(kernel_size, kernel_size), 0);
    // 裁剪回原始 ROI 尺寸
    cv::Rect crop_roi(
	overlap - (roi.x - extended_roi.x),
	overlap - (roi.y - extended_roi.y),
	roi.width,
	roi.height
    );
    return blurred(crop_roi).clone();
}

// TBB 分块并行高斯模糊主函数
void tbbParallelBlur(const cv::Mat& src, cv::Mat& dst, int kernel_size, int num_blocks_x, int num_blocks_y) {
    dst = cv::Mat::zeros(src.size(), src.type());
    int block_w = src.cols / num_blocks_x;
    int block_h = src.rows / num_blocks_y;

    tbb::parallel_for(tbb::blocked_range2d<int>(0, num_blocks_y, 0, num_blocks_x),
	[&](const tbb::blocked_range2d<int>& r) {
	    for (int i = r.rows().begin(); i < r.rows().end(); ++i) {
		for (int j = r.cols().begin(); j < r.cols().end(); ++j) {
		    cv::Rect roi(j * block_w, i * block_h, block_w, block_h);
		    // 最后一行/列可能超出，修正
		    if (i == num_blocks_y - 1) roi.height = src.rows - i * block_h;
		    if (j == num_blocks_x - 1) roi.width = src.cols - j * block_w;
		    cv::Mat blurred_block = blurBlockWithOverlap(src, roi, kernel_size);
		    blurred_block.copyTo(dst(roi));
		}
	    }
	}
    );
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./tbb_block_blur <image_path>" << std::endl;
        return -1;
    }
    cv::Mat img = cv::imread(argv[1]);
    if (img.empty()) {
	std::cerr << "Cannot read image" << std::endl;
        return -1;
    }

    int kernel = 15;          // 高斯核大小
    int blocks_x = 8, blocks_y = 8;  // 分块数（可根据图像大小调整）
    
    // TBB 并行分块版本
    cv::Mat result_tbb;
    auto start = std::chrono::steady_clock::now();
    tbbParallelBlur(img, result_tbb, kernel, blocks_x, blocks_y);
    auto end = std::chrono::steady_clock::now();
    double tbb_time = std::chrono::duration<double>(end - start).count();

     // 串行版本（直接对整个图像高斯模糊）
    cv::Mat result_serial;
    start = std::chrono::steady_clock::now();
    cv::GaussianBlur(img, result_serial, cv::Size(kernel, kernel), 0);
    end = std::chrono::steady_clock::now();
    double serial_time = std::chrono::duration<double>(end - start).count();

    std::cout << "Serial GaussianBlur time: " << serial_time << " s" << std::endl;
    std::cout << "TBB block parallel time: " << tbb_time << " s" << std::endl;
    std::cout << "Speedup: " << serial_time / tbb_time << std::endl;

    // 显示结果差异（确保正确性）
    cv::Mat diff;
    cv::absdiff(result_serial, result_tbb, diff);
    double max_diff;
    cv::minMaxLoc(diff, nullptr, &max_diff);
    std::cout << "Max difference between serial and TBB result: " << max_diff << std::endl;

    cv::imwrite("result_tbb_blur.jpg", result_tbb);
    return 0;
}
