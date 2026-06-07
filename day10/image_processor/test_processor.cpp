#include "image_processor.h"
#include <iostream>

int main(int argc, char** argv) {
    cv::Mat img = cv::imread("test.jpg");
    if (img.empty()) {
	std::cerr << "无法读取图像" << std::endl;
	return -1;
    }

    // 使用 uchar 类型（8位图像）
    try {
	auto processor = makeImageProcessor<uchar>(img);

	auto result = processor.toGray()
			       .resize(400, 300)
			       .gaussianBlur(5)
			       .canny(50, 150);

	processor.show("Original", 0);
	result.show("Processed", 0);
	result.save("output.jpg");

	std::cout << "处理完成，输出图像已保存" << std::endl;
	
    } catch (const std::exception& e) {
	std::cerr << "错误：" << e.what() << std::endl;
    }

    return 0;
}
