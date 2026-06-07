#include "image_processor_advanced.h"
#include <iostream>

int main() {
    // 1. 使用普通 Mat，像素类型 unsigned char
    cv::Mat img = cv::imread("test.jpg");
    if (img.empty()) {
	std::cerr << "无法读取图片" << std::endl;
        return -1;
    }

    // 2. 创建延迟处理器（使用 uchar，不使用 UMat）
    ImageProcessorLazy<unsigned char, false> processor(img);

    processor.addOperation(ImageOps::toGray<unsigned char, false>())
	     .addOperation(ImageOps::resize<unsigned char, false>(400, 300))
	     .addOperation(ImageOps::equalizeHist<unsigned char, false>())
	     .addOperation(ImageOps::morphologyOpen<unsigned char, false>(3))
	     .addOperation(ImageOps::canny<unsigned char, false>(50, 150));

    // 4. 执行并显示
    processor.show("Result", 0);
    processor.save("output_advanced.jpg");

    std::cout << "处理完成，结果已保存" << std::endl;

    // 5. 可选：使用 UMat 加速（需要 OpenCL 支持）
#ifndef CV_OPENCL_RUN
    cv::UMat uimg = img.getUMat(cv::ACCESS_READ);
    ImageProcessorLazy<unsigned char, true> uprocessor(uimg);
    uprocessor.addOperation(ImageOps::gaussianBlur<unsigned char, true>(5))
	      .addOperation(ImageOps::resize<unsigned char, true>(640, 480))
	      .addOperation(ImageOps::rotate<unsigned char, true>(90));
    cv::UMat uresult = uprocessor.execute();
    cv::imshow("UMat Result", uresult);
    cv::waitKey(0);
#endif

    return 0;
}
