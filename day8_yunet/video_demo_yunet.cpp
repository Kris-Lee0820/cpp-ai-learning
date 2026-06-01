#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <iostream>
#include <chrono>

// ========== 全局变量 ==========
cv::Mat g_frame;
cv::Mat g_display_frame;

// 亮度/对比度/Canny
int g_brightness = 50;
int g_contrast = 100;
int g_edge_mode = 0;
int g_canny_low = 50;
int g_canny_high = 150;

// 人脸检测
bool g_face_detection_enabled = true;
cv::Ptr<cv::FaceDetectorYN> g_detector;  // YuNet 检测器
int g_yunet_score_threshold = 50;    // 0~100 映射到 0.5 阈值

// FPS
std::chrono::steady_clock::time_point g_last_time;
double g_fps = 0.0;

// ========== 函数声明 ==========
void processFrame();
void onBrightnessChange(int, void*) { processFrame(); }
void onContrastChange(int, void*)   { processFrame(); }
void onEdgeModeChange(int, void*)   { processFrame(); }
void onCannyLowChange(int, void*)   { processFrame(); }
void onCannyHighChange(int, void*)  { processFrame(); }
void onYuNetScoreChange(int, void*) { processFrame(); }

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

    // 1. BGR 转换
    cv::Mat bgr_frame = ensureBGR(g_frame);

    // 2. 亮度对比度
    int brightness_offset = g_brightness - 50;
    double contrast_alpha = g_contrast / 100.0;
    cv::Mat adjusted;
    bgr_frame.convertTo(adjusted, -1, contrast_alpha, brightness_offset);

    // 3. 边缘检测或原图
    if (g_edge_mode == 1) {
        cv::Mat gray, edges;
        cv::cvtColor(adjusted, gray, cv::COLOR_BGR2GRAY);
        cv::Canny(gray, edges, g_canny_low, g_canny_high);
        cv::cvtColor(edges, g_display_frame, cv::COLOR_GRAY2BGR);
    } else {
        g_display_frame = adjusted.clone();
    }

    // 4. 人脸检测（仅彩色模式，且启用人脸检测）
    std::vector<cv::Rect> faces;
    std::vector<float> confidences;

    if (g_face_detection_enabled && g_edge_mode == 0 && !g_detector.empty()) {
        // 关键：每次检测前设置输入尺寸为当前图像尺寸
        g_detector->setInputSize(adjusted.size());
        cv::Mat detections;
        g_detector->detect(adjusted, detections);
        
        // 解析 YuNet 输出: [x, y, w, h, ..., score]
        float score_threshold = g_yunet_score_threshold / 100.0f;
        for (int i = 0; i < detections.rows; ++i) {
            float* data = detections.ptr<float>(i);
            float score = data[14];  // 置信度索引
            if (score > score_threshold) {
                int x = static_cast<int>(data[0]);
                int y = static_cast<int>(data[1]);
                int w = static_cast<int>(data[2]);
                int h = static_cast<int>(data[3]);
                faces.push_back(cv::Rect(x, y, w, h));
                confidences.push_back(score);
            }
        }
    }

    // 5. 绘制人脸框和置信度
    for (size_t i = 0; i < faces.size(); ++i) {
        cv::rectangle(g_display_frame, faces[i], cv::Scalar(0, 255, 0), 2);
        std::string text = cv::format("%.2f", confidences[i]);
        cv::putText(g_display_frame, text, cv::Point(faces[i].x, faces[i].y - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
    }

    // 6. FPS 计算
    auto now = std::chrono::steady_clock::now();
    if (g_last_time.time_since_epoch().count() != 0) {
        double elapsed = std::chrono::duration<double>(now - g_last_time).count();
        if (elapsed > 0) g_fps = 0.9 * g_fps + 0.1 / elapsed;
    }
    g_last_time = now;

    // 7. 显示参数
    int line = 30, step = 25;
    cv::putText(g_display_frame, cv::format("FPS: %.1f", g_fps), cv::Point(10, line),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
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
    if (g_face_detection_enabled && g_edge_mode == 0) {
        cv::putText(g_display_frame, "Face Detect: ON (YuNet)", cv::Point(10, line),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 200, 0), 1);
        line += step;
        cv::putText(g_display_frame, cv::format("Score Thresh: %.2f", g_yunet_score_threshold / 100.0),
                    cv::Point(10, line), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 200, 0), 1);
    } else if (g_face_detection_enabled) {
        cv::putText(g_display_frame, "Face Detect: ON (disabled in edge mode)", cv::Point(10, line),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(100, 100, 100), 1);
    } else {
        cv::putText(g_display_frame, "Face Detect: OFF", cv::Point(10, line),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(200, 200, 200), 1);
    }

    cv::imshow("Video", g_display_frame);
}

int main(int argc, char** argv) {
    // 打开视频
    cv::VideoCapture cap;
    if (argc > 1) cap.open(argv[1]);
    else cap.open(0);
    if (!cap.isOpened()) {
        std::cerr << "错误：无法打开视频源" << std::endl;
        return -1;
    }

    // 加载 YuNet 模型（不指定输入尺寸，后续动态设置）
    std::string model_path = "face_detection_yunet_2023mar.onnx";
    g_detector = cv::FaceDetectorYN::create(model_path, "", cv::Size(0, 0));
    if (g_detector.empty()) {
        std::cerr << "警告：YuNet 模型加载失败，人脸检测禁用" << std::endl;
        g_face_detection_enabled = false;
    } else {
        std::cout << "YuNet loaded." << std::endl;
    }

    cv::namedWindow("Video", cv::WINDOW_NORMAL);
    cv::createTrackbar("Brightness", "Video", &g_brightness, 100, onBrightnessChange);
    cv::createTrackbar("Contrast",   "Video", &g_contrast,   200, onContrastChange);
    cv::createTrackbar("Edge Mode",  "Video", &g_edge_mode,   1,  onEdgeModeChange);
    cv::createTrackbar("Canny Low",  "Video", &g_canny_low,   255, onCannyLowChange);
    cv::createTrackbar("Canny High", "Video", &g_canny_high,  255, onCannyHighChange);
    cv::createTrackbar("Score Thresh","Video", &g_yunet_score_threshold, 100, onYuNetScoreChange);

    std::cout << "Controls: ESC exit, SPACE pause, f toggle face detection" << std::endl;

    while (true) {
        cap >> g_frame;
        if (g_frame.empty()) {
            if (argc <= 1) break;
            std::cout << "视频循环..." << std::endl;
            cap.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }
        processFrame();

        char key = (char)cv::waitKey(30);
        if (key == 27) break;
        if (key == ' ') cv::waitKey(0);
        if (key == 'f') {
            g_face_detection_enabled = !g_face_detection_enabled;
            processFrame();
        }
    }
    return 0;
}
