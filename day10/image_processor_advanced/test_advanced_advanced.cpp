#include "image_processor_advanced_advanced.h"
#include <iostream>

int main() {
    // 读取图像
    cv::Mat img = cv::imread("test.jpg");
    if (img.empty()) {
        std::cerr << "无法读取 test.jpg" << std::endl;
        return -1;
    }

    // 1. 普通用法：CLAHE + 形态学操作
    {
        ImageProcessorLazy<unsigned char, false> processor(img);
        processor.addOperation(ImageOps::toGray<unsigned char, false>())
                 .addOperation(ImageOps::clahe<unsigned char, false>(2.0, cv::Size(8,8)))
                 .addOperation(ImageOps::resize<unsigned char, false>(400, 300))
                 .addOperation(ImageOps::gaussianBlur<unsigned char, false>(5));
        processor.show("CLAHE Result", 0);
        processor.save("clahe_output.jpg");
    }

    // 2. 条件操作：如果图像平均亮度 < 128，则进行直方图均衡化，否则高斯模糊
    {
        ImageProcessorLazy<unsigned char, false> processor(img);
        auto brightnessCondition = [](const cv::Mat& img) {
            return ImageOps::meanBrightness<unsigned char, false>(img) < 128;
        };
        auto enhanceOp = ImageOps::equalizeHist<unsigned char, false>();
        auto blurOp = ImageOps::gaussianBlur<unsigned char, false>(5);
        auto conditionalOp = ImageOps::ifThenElse<unsigned char, false>(brightnessCondition, enhanceOp);
        // 注意：ifThenElse 返回的操作本身也是一个操作，可以链式添加
        processor.addOperation(conditionalOp)
                 .addOperation(blurOp);  // 无论条件如何，最后都加一个模糊
        processor.show("Conditional Result", 0);
        processor.save("conditional_output.jpg");
    }

    // 3. 支持 float 图像
    {
        cv::Mat floatImg;
        img.convertTo(floatImg, CV_32F, 1.0/255.0);
        ImageProcessorLazy<float, false> processor(floatImg);
        processor.addOperation(ImageOps::toGray<float, false>())
                 .addOperation(ImageOps::canny<float, false>(50, 150))
                 .addOperation(ImageOps::resize<float, false>(400, 300));
        processor.show("Float Image Processed", 0);
        processor.save("float_output.jpg");
    }

    return 0;
}
