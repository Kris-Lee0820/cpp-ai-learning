#include <opencv2/opencv.hpp>
#include <tbb/tbb.h>
#include <chrono>

void adjustBrightnessTBB(cv::Mat& img, int delta) {
    // 1. 创建二维分块范围，覆盖整个图像的行和列
    auto range = tbb::blocked_range2d<int>(0, img.rows, 0, img.cols);
    // 2. TBB 并行循环：自动将 range 分割成多个子块，每个子块交给一个线程处理
    tbb::parallel_for(range, [&](const tbb::blocked_range2d<int>& r) {
	// r 是当前线程负责的一个子块（例如行区间 [r.rows().begin(), r.rows().end())，
        // 列区间 [r.cols().begin(), r.cols().end())）
	
	// 遍历当前子块内的所有行
	for (int i = r.rows().begin(); i < r.rows().end(); ++i) {
	     // 获取第 i 行的首地址，类型为 cv::Vec3b*（每个元素是一个包含 3 个 uchar 的向量）
	    
	    //  2. 像素访问方式
	    // img.ptr<cv::Vec3b>(i) 返回第 i 行首指针，类型为 cv::Vec3b*。
      	    // row[j][c] 等价于访问第 i 行、第 j 列、第 c 通道的像素值。
            //  这种访问比 img.at<cv::Vec3b>(i,j)[c] 更高效（减少了索引计算和边界检查），适合循环内使用	
	    cv::Vec3b* row = img.ptr<cv::Vec3b>(i);

	    // 遍历当前子块内的列
	    for (int j = r.cols().begin(); j < r.cols().end(); ++j) {
		// 对每个像素的三个通道分别处理
		for (int c = 0; c < 3; ++c) {
		    int v = row[j][c] + delta;                  // 计算新值
		    
		    // 3. 饱和转换 cv::saturate_cast<uchar>
		    // 当 delta 为正数时，值可能超过 255；为负数时可能低于 0。
		    // saturate_cast<uchar> 自动将超出范围的值截断到 [0,255]，例如：
		    // 输入 300 → 255
		    // 输入 -5 → 0
		    // 这保证了图像数据始终有效。
		    row[j][c] = cv::saturate_cast<uchar>(v);	// 饱和转换到 0~255
		}
	    }
	}	
    });
}

int main() {
    cv::Mat img = cv::imread("large.jpg");
    auto start = std::chrono::steady_clock::now();
    adjustBrightnessTBB(img, 50);
    auto end = std::chrono::steady_clock::now();
    std::cout << "TBB time: " << std::chrono::duration<double>(end - start).count() << " s" << std::endl;
    cv::imwrite("result_tbb.jpg", img);
    return 0;
}
