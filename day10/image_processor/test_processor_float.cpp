#include "image_processor.h"
#include <iostream>

int main() {
    // 1. 读取普通 8UC3 图像
    cv::Mat img = cv::imread("test.jpg");
    if (img.empty()) {
	std::cerr << "无法读取 test.jpg" << std::endl;
	return -1;
    }

    // 2. 将图像转坏为 float 类型（范围0.0~1.0）
    cv::Mat float_img;
    img.convertTo(float_img, CV_32F, 1.0 / 255.0);

    // 3. 创建 float 类型的处理器
    std::cout << "float_img depth is : " << float_img.depth() << std::endl;
    auto processor = makeImageProcessor<float>(float_img);

    // 4. 应用处理链（灰度 -> 缩放 -> 高斯模糊 -> Canny）
    auto result = processor.toGray()
	    		   .resize(400, 300)
			   .gaussianBlur(5)
			   .canny(50, 150);

    // 5.显示原图（转换回 8U 显示）
    cv::Mat display_original;
    float_img.convertTo(display_original, CV_8U, 255.0);
    cv::imshow("Original (Float -> 8U)", display_original);
    result.show("Processed (Float)", 0);
    result.save("output_float.jpg");

    std::cout << "Float 图像处理完成，结果已保存" << std::endl;
    return 0;
}
