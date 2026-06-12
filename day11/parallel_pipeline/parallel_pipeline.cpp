#include <opencv2/opencv.hpp>
#include <tbb/tbb.h>
#include <oneapi/tbb/parallel_pipeline.h>
#include <oneapi/tbb/task_arena.h>
#include <iostream>

cv::Mat processFrame(const cv::Mat& frame) {
    cv::Mat result;
    cv::GaussianBlur(frame, result, cv::Size(15,15), 0);
    return result;
}

int main() {
    cv::VideoCapture cap("test.mp4");
    if (!cap.isOpened()) return -1;
    
    auto start = std::chrono::steady_clock::now();
    int frame_count = 0;
    
    tbb::parallel_pipeline(
        tbb::task_arena::automatic,  // 使用默认资源
        tbb::make_filter<void, cv::Mat>(
            tbb::filter_mode::serial_in_order,      // 输入阶段串行（读取视频帧）
            [&](tbb::flow_control& fc) -> cv::Mat {
                cv::Mat frame;
                cap >> frame;
                if (frame.empty()) {
                    fc.stop();
                    return cv::Mat();
                }
                return frame;
            }
        ) &
        oneapi::tbb::make_filter<cv::Mat, cv::Mat>(
            tbb::filter_mode::parallel,    // 处理阶段并行
            [](cv::Mat frame) {
                return processFrame(frame);
            }
        ) &
        oneapi::tbb::make_filter<cv::Mat, void>(
            tbb::filter_mode::serial_in_order,      // 输出阶段串行（显示）
            [&](cv::Mat processed) {
                cv::imshow("Pipeline", processed);
                frame_count++;
                if (cv::waitKey(1) == 27) {
                    // 无法直接停止 pipeline，但可以设置全局标志
                }
            }
        )
    );
    
    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    std::cout << "Processed " << frame_count << " frames in " << elapsed << " seconds ("
              << frame_count/elapsed << " FPS)" << std::endl;
    return 0;
}
