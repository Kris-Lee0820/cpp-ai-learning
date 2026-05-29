#include <opencv2/opencv.hpp>
#include <iostream>

// 全局变量用于滑动条回调
int brightness = 100;     // 亮度初始值（0-200，100 表示原亮度）
cv::Mat frame;

// 滑动条回调函数，当滑动条改变时调用
void onBrightnessChange(int val, void* userdata) {
    // val 的范围是0~200，我们映射到 -100~+100 的偏移
    int offset = val - 100;
    // 对图形进行亮度调整：每个像素加上偏移值
    cv::Mat adjusted;
    frame.convertTo(adjusted, -1, 1, offset); //参数：src，type，alpha，beta
    cv::imshow("Video", adjusted);
}

void onTrackbar(int, void* userdata) {
    brightness = cv::getTrackbarPos("Brightness", "Video");
}

int main(int argc, char** argv) {
    // 1. 打开视频源：摄像头（0）或视频文件
    cv::VideoCapture cap;
    if (argc > 1) {
        cap.open(argv[1]); // 从命令行指定视频文件
    } else {
	cap.open(0);	   // 默认摄像头
    }

    if (!cap.isOpened()) {
	std::cerr << "ERROR: Could not open camera or file" << std::endl;
	return -1;
    }

    // 2. 创建窗口并绑定滑动条
    cv::namedWindow("Video", cv::WINDOW_NORMAL);
    cv::createTrackbar("Brightness", "Video", NULL, 100, onTrackbar);

    // 3. 循环读取帧
    while (true) {
	cap >> frame;
	if (frame.empty()) break;

	// 调用滑动条回调手动处理当前帧（也可以把调整代码写在这里）
	onBrightnessChange(brightness, nullptr);

	// 等待按键，按 ESC 退出
	char key = cv::waitKey(30);
	if (key == 27) break; // 27 是 ESC 的 ASCII 码
    }

    // 4. 释放资源
    cap.release();
    cv::destroyAllWindows();
    return 0;
}
