#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

// ========== 全局变量 ==========
// 图像处理
cv::Mat g_frame;
cv::Mat g_display_frame;

// 亮度/对比度/Canny
int g_brightness = 50;		// 0-100 -> -50..+50
int g_contrast = 100;		// 0-200 -> 0.5..2.0
int g_edge_mode = 0;		// 0=彩色, 1=Canny边缘
int g_canny_low = 50;
int g_canny_high = 150;

// 人脸检测
bool g_face_detection_enabled = true;
cv::CascadeClassifier g_face_cascade;
cv::CascadeClassifier g_eye_cascade;
int g_haar_scaleFactor_int = 105;	// 对用1.05
double g_haar_scaleFactor = 1.05;
int g_haar_minNeighbors = 5;
int g_haar_minSize = 50;		// 最小人脸尺寸

// FPS 计算
std::chrono::steady_clock::time_point g_last_time;
double g_fps = 0.0;

// ========== 函数声明 ==========
void processFrame();
void onBrightnessChange(int, void*) { processFrame(); }
void onContrastChange(int, void*)   { processFrame(); }
void onEdgeModeChange(int, void*)   { processFrame(); }
void onCannyLowChange(int, void*)   { processFrame(); }
void onCannyHighChange(int, void*)  { processFrame(); }
void onFaceScaleChange(int, void*) {
    if (g_haar_scaleFactor_int < 101) g_haar_scaleFactor_int = 101;
    g_haar_scaleFactor = g_haar_scaleFactor_int / 100.0;
    processFrame();
}
void onFaceNeighborsChange(int, void*) {
    // g_haar_minNeighbors 已自动更新
    processFrame();
}
void onFaceMinSizeChange(int, void*) {
    processFrame();
}

cv::Mat ensureBGR(const cv::Mat& src) {
    if (src.channels() == 3) return src.clone();
    if (src.channels() == 1) {
	cv::Mat bgr;
	cv::cvtColor(src, bgr, cv::COLOR_GRAY2BGR);
	return bgr;
    }
    cv::Mat bgr;
    cv::cvtColor(src, bgr, cv::COLOR_BGRA2BGR);
    return bgr;
}

void processFrame() {
    if (g_frame.empty()) return;

    // 1. 确保 BGR 三通道
    cv::Mat bgr_frame = ensureBGR(g_frame);

    // 2. 亮度对比度调整
    int brightness_offset = g_brightness - 50;
    double contrast_alpha = g_contrast / 100.0;
    cv::Mat adjusted;
    bgr_frame.convertTo(adjusted, -1, contrast_alpha, brightness_offset);

    // 3. 边缘检测或直接显示
    if (g_edge_mode == 1) {
	cv::Mat gray, edges;
	cv::cvtColor(adjusted, gray, cv::COLOR_BGR2GRAY);
	cv::Canny(gray, edges, g_canny_low, g_canny_high);
	cv::cvtColor(edges, g_display_frame, cv::COLOR_GRAY2BGR);
    } else {
	g_display_frame = adjusted.clone();
    }

    // 4. 人脸检测（仅在彩色模式且启用人脸检测）
    std::vector<cv::Rect> faces;
    if (g_face_detection_enabled && !g_face_cascade.empty() && g_edge_mode == 0) {
	cv::Mat gray;
	cv::cvtColor(adjusted, gray, cv::COLOR_BGR2GRAY);
	// 直方图均衡化提升对比度
	cv::equalizeHist(gray, gray);
	g_face_cascade.detectMultiScale(gray, faces,
					g_haar_scaleFactor,
					g_haar_minNeighbors,
					0,
					cv::Size(g_haar_minSize, g_haar_minSize));
    }

    // 5. 背景模糊（马赛克效果）: 如果检测到人脸且非边缘模式
    if (g_face_detection_enabled && !faces.empty() && g_edge_mode == 0) {
	cv::Mat blurred;
	cv::GaussianBlur(g_display_frame, blurred, cv::Size(31, 31), 0);
	g_display_frame = blurred.clone();
	// 将清晰人脸区域从 adjusted 复制回来
	for (const auto& face : faces) {
	    cv::Rect roi = face & cv::Rect(0, 0, g_display_frame.cols, g_display_frame.rows);            cv::Mat face_roi = adjusted(roi);
	    face_roi.copyTo(g_display_frame(roi));
	}
    }

    // 6. 绘制人脸框和眼睛框（在显示帧上）
    for (const auto& face : faces) {
	cv::rectangle(g_display_frame, face, cv::Scalar(0, 255, 0), 2);

	// 眼睛检测（仅当模型加载成功且非边缘模式）
	if (!g_eye_cascade.empty() && g_edge_mode == 0) {
	    cv::Mat face_roi = g_display_frame(face);
	    cv::Mat face_roi_gray;
	    cv::cvtColor(face_roi, face_roi_gray, cv::COLOR_BGR2GRAY);
	    std::vector<cv::Rect> eyes;
	    g_eye_cascade.detectMultiScale(face_roi_gray, eyes, 1.05, 8);
	    for (const auto& eye : eyes) {
		cv::Rect eye_abs(face.x + eye.x, face.y + eye.y, eye.width, eye.height);
		cv::rectangle(g_display_frame, eye_abs, cv::Scalar(255, 0, 0), 2);
	    }
	}
    }

    // 7. 计算 FPS
    auto now = std::chrono::steady_clock::now();
    if (g_last_time.time_since_epoch().count() == 0) {
	g_last_time = now;
    } else {
	double elapsed = std::chrono::duration<double>(now - g_last_time).count();
	if (elapsed > 0) {
	    g_fps = 0.9 * g_fps + 0.1 * (1.0 / elapsed);
	}
	g_last_time = now;
    }

    // 8. 在图像上显示参数
    int line = 30;
    int step = 25;
    cv::putText(g_display_frame, cv::format("FPS: %.1f", g_fps), cv::Point(10, line), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
    line += step;
    cv::putText(g_display_frame, cv::format("Brightness: %d", g_brightness), cv::Point(10, line),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 1);
    line += step;
    cv::putText(g_display_frame, cv::format("Contrast: %d", g_contrast), cv::Point(10, line),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 1);
    line += step;
    cv::putText(g_display_frame, cv::format("Edge Mode: %s", g_edge_mode ? "Canny" : "Color"), cv::Point(10, line),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 1);
    line += step;
    if (g_edge_mode == 1) {
        cv::putText(g_display_frame, cv::format("Canny Low/High: %d/%d", g_canny_low, g_canny_high), cv::Point(10, line),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 1);
        line += step;
    }
    if (g_edge_mode == 1) {
        cv::putText(g_display_frame, cv::format("Canny Low/High: %d/%d", g_canny_low, g_canny_high), cv::Point(10, line),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 1);
        line += step;
    }
    if (g_face_detection_enabled && g_edge_mode == 0) {
        cv::putText(g_display_frame, cv::format("Face Detect: ON"), cv::Point(10, line),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 200, 0), 1);
        line += step;
        cv::putText(g_display_frame, cv::format("Scale: %.2f  Neighbors: %d  MinSize: %d",
                    g_haar_scaleFactor, g_haar_minNeighbors, g_haar_minSize), cv::Point(10, line),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 200, 0), 1);
        line += step;
    } else if (g_face_detection_enabled) {
        cv::putText(g_display_frame, "Face Detect: ON (disabled in edge mode)", cv::Point(10, line),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(100, 100, 100), 1);
        line += step;
    } else {
        cv::putText(g_display_frame, "Face Detect: OFF", cv::Point(10, line),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(200, 200, 200), 1);
        line += step;
    }

    cv::imshow("Video", g_display_frame);
}

int main(int argc, char** argv) {
    // 打开视频源
    cv::VideoCapture cap;
    if (argc > 1) {
	cap.open(argv[1]);
    } else {
	cap.open(0);	// 摄像头
    }
    if (!cap.isOpened()) {
	std::cerr << "错误：无法打开视频源" << std::endl;
	return -1;
    }

    // 加载 Haar 级联分类器
    std::string face_cascade_path = "haarcascade_frontalface_default.xml";
    if (!g_face_cascade.load(face_cascade_path)) {
	std::cerr << "警告：无法加载人脸模型文件 " << face_cascade_path << "，人脸检测不可用" << std::endl;
        g_face_detection_enabled = false;
    }
    std::string eye_cascade_path = "haarcascade_eye.xml";
    if (!g_eye_cascade.load(eye_cascade_path)) {
        std::cerr << "警告：无法加载眼睛模型文件 " << eye_cascade_path << "，眼睛检测不可用" << std::endl;
    }

    // 创建窗口和滑动条
    cv::namedWindow("Video", cv::WINDOW_NORMAL);
    cv::createTrackbar("Brightness", "Video", &g_brightness, 100, onBrightnessChange);
    cv::createTrackbar("Contrast",   "Video", &g_contrast,   200, onContrastChange);
    cv::createTrackbar("Edge Mode",  "Video", &g_edge_mode,   1,  onEdgeModeChange);
    cv::createTrackbar("Canny Low",  "Video", &g_canny_low,   255, onCannyLowChange);
    cv::createTrackbar("Canny High", "Video", &g_canny_high,  255, onCannyHighChange);
    cv::createTrackbar("Face Scale", "Video", &g_haar_scaleFactor_int, 200, onFaceScaleChange);
    cv::createTrackbar("Face Neighbors", "Video", &g_haar_minNeighbors, 20, onFaceNeighborsChange);
    cv::createTrackbar("Face MinSize", "Video", &g_haar_minSize, 200, onFaceMinSizeChange);
    // 设置初始值
    cv::setTrackbarPos("Brightness", "Video", g_brightness);
    cv::setTrackbarPos("Contrast", "Video", g_contrast);
    cv::setTrackbarPos("Edge Mode", "Video", g_edge_mode);
    cv::setTrackbarPos("Canny Low", "Video", g_canny_low);
    cv::setTrackbarPos("Canny High", "Video", g_canny_high);
    cv::setTrackbarPos("Face Scale", "Video", g_haar_scaleFactor_int);
    cv::setTrackbarPos("Face Neighbors", "Video", g_haar_minNeighbors);
    cv::setTrackbarPos("Face MinSize", "Video", g_haar_minSize);

    std::cout << "按键说明:\n"
              << "  ESC - 退出\n"
              << "  空格 - 暂停\n"
              << "  f - 开关人脸检测\n"
              << "  滑动条实时调节参数\n";

    // 主循环
    while (true) {
	cap >> g_frame;
	if (g_frame.empty()) {
	    if (argc < 1) break;	// 摄像头无帧则退出
	    std::cout << "视频播放完毕，重新循环..." << std::endl;
	    cap.set(cv::CAP_PROP_POS_FRAMES, 0);
	    continue;
	}
	processFrame();

	char key = (char)cv::waitKey(30);
	if (key == 27) break;       // ESC 退出
        if (key == ' ') cv::waitKey(0);   // 空格暂停
        if (key == 'f') {
            g_face_detection_enabled = !g_face_detection_enabled;
            processFrame();
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
