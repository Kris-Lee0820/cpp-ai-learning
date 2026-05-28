#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    //读取图片
    cv::Mat img = cv::imread("test.jpg");
    if (img.empty()) {
	std::cerr << "无法读取图片" << std::endl;
	return -1;
    }

    //显示原图
    cv::imshow("original", img);

    //转为灰度图片
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::imshow("Gray", gray);
    cv::imwrite("gray.jpg", gray); //保存灰度图片
				   

    //缩放到300x300
    cv::Mat resized;
    cv::resize(img, resized, cv::Size(300, 300));
    cv::imshow("Resized", resized);

    std::cout << "按任意按键关闭窗口..." << std::endl;
    cv::waitKey(0); //等待按键 
    return 0;
}
