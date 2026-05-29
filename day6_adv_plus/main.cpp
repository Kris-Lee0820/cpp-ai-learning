#include <opencv2/opencv.hpp>
#include <iostream>

// 全局变量
int g_brightness = 50;      // 亮度 0~100 -> -50~+50
int g_contrast = 100;       // 对比度 0~200 -> 0.5~2.0
int g_edge_mode = 0;        // 0=彩色, 1=Canny边缘
int g_canny_low = 50;       // Canny 低阈值
int g_canny_high = 150;     // Canny 高阈值


cv::Mat g_frame;
cv::Mat g_display_frame;

// FPS 计算相关
int64 g_tick = 0;
double g_fps = 0.0;

// 函数声明
void processFrame();

// 确保图像是 3 通道 BGR（如果是灰度或RGBA则转换）
cv::Mat ensureBGR(const cv::Mat& src) {
    if (src.channels() == 3) return src.clone();
    if (src.channels() == 1) {
        cv::Mat bgr;
        cv::cvtColor(src, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    // 其他情况（如4通道RGBA）可转为BGR
    cv::Mat bgr;
    cv::cvtColor(src, bgr, cv::COLOR_BGRA2BGR);
    return bgr;
}

void onBrightnessChange(int, void*) { processFrame(); }
void onContrastChange(int, void*)   { processFrame(); }
void onEdgeModeChange(int, void*)   { processFrame(); }
void onCannyLowChange(int, void*)   { processFrame(); }
void onCannyHighChange(int, void*)  { processFrame(); }

void processFrame() {
    if (g_frame.empty()) return;

    // 确保输入是 BGR 三通道
    cv::Mat bgr_frame = ensureBGR(g_frame);

    // 亮度偏移 (-50~+50)
    int brightness_offset = g_brightness - 50;
    // 对比度倍数 (0.5~2.0)
    double contrast_alpha = g_contrast / 100.0;

    cv::Mat temp;
    bgr_frame.convertTo(temp, -1, contrast_alpha, brightness_offset);

    if (g_edge_mode == 1) {
        cv::Mat gray, edges;
        cv::cvtColor(temp, gray, cv::COLOR_BGR2GRAY);
        cv::Canny(gray, edges, g_canny_low, g_canny_high);
        cv::cvtColor(edges, g_display_frame, cv::COLOR_GRAY2BGR);
    } else {
        g_display_frame = temp;
    }
    // 在图像左上角显示 FPS
    if (g_tick == 0) {
	g_tick = cv::getTickCount();
    } else {
     	int64 new_tick = cv::getTickCount();
	double elapsed = (new_tick - g_tick) / cv::getTickFrequency();
	if (elapsed > 0) {
	    g_fps = 0.9 * g_fps + 0.1 / elapsed; // 低通滤波，平滑显示
	}
	g_tick = new_tick;
    }
    std::string fps_text = cv::format("FPS: %.1f", g_fps);
    cv::putText(g_display_frame, fps_text, cv::Point(15, 30),
		cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

    cv::imshow("Video", g_display_frame);
}

int main(int argc, char** argv) {
    cv::VideoCapture cap;
    if (argc > 1) {
        cap.open(argv[1]);
    } else {
        cap.open(0);   // 摄像头
    }

    if (!cap.isOpened()) {
        std::cerr << "错误：无法打开视频源" << std::endl;
        return -1;
    }

    cv::namedWindow("Video", cv::WINDOW_NORMAL);

    // 创建滑动条（直接使用全局变量指针）
    cv::createTrackbar("Brightness", "Video", &g_brightness, 100, onBrightnessChange);
    cv::createTrackbar("Contrast",   "Video", &g_contrast,   200, onContrastChange);
    cv::createTrackbar("Edge Mode",  "Video", &g_edge_mode,   1,  onEdgeModeChange);
    cv::createTrackbar("Canny Low",  "Video", &g_canny_low,  255, onCannyLowChange);
    cv::createTrackbar("Canny High", "Video", &g_canny_high, 255, onCannyHighChange);

    std::cout << "操作说明：\n"
              << "  Brightness: 0~100 (默认50)\n"
              << "  Contrast:   0~200 (默认100)\n"
              << "  Edge Mode:  0=彩色, 1=Canny边缘\n"
              << "按键：ESC-退出, 空格-暂停, r-重置参数\n" << std::endl;

    while (true) {
        cap >> g_frame;

        // 视频结束时循环播放（仅当读取的是文件时）
        if (g_frame.empty()) {
            // 如果是摄像头（通常不会空），则退出；否则重置视频到开头
            if (argc <= 1) break;   // 摄像头无帧则退出
            std::cout << "视频播放完毕，重新循环..." << std::endl;
            cap.set(cv::CAP_PROP_POS_FRAMES, 0);  // 重置到第一帧
            continue;   // 跳过本次循环，重新读取
        }

        processFrame();

        char key = (char)cv::waitKey(30);
        if (key == 27) break;       // ESC 退出
        if (key == ' ') {
            cv::waitKey(0);         // 空格暂停
        }
        if (key == 'r') {           // 重置参数
            g_brightness = 50;
            g_contrast = 100;
            g_edge_mode = 0;
	    g_canny_low = 50;
	    g_canny_high = 150;

            cv::setTrackbarPos("Brightness", "Video", g_brightness);
            cv::setTrackbarPos("Contrast",   "Video", g_contrast);
            cv::setTrackbarPos("Edge Mode",  "Video", g_edge_mode);
	    cv::setTrackbarPos("Canny Low",  "Video", g_canny_low);
	    cv::setTrackbarPos("Canny High", "Video", g_canny_high);

            processFrame();         // 立即刷新
	    std::cout << "参数已重置" << std::endl;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
