#include <opencv2/opencv.hpp>
#include "BlockingQueue.h"
#include <thread>
#include <atomic>

cv::Mat processFrame(const cv::Mat& frame) {
    cv::Mat result;
    cv::GaussianBlur(frame, result, cv::Size(15, 15), 0);
    return result;
}

int main() {
    cv::VideoCapture cap("test.mp4");
    if (!cap.isOpened()) return -1;

    const int QUEUE_SIZE = 10;
    BlockingQueue<cv::Mat> raw_queue(QUEUE_SIZE);
    BlockingQueue<cv::Mat> processed_queue(QUEUE_SIZE);
    std::atomic<bool> stop{false};

    // 读取线程
    std::thread reader([&]() {
        while (!stop) {
            cv::Mat frame;
            cap >> frame;
            if (frame.empty()) break;
            raw_queue.push(frame);
        }
        // 发送空帧作为结束信号
        raw_queue.push(cv::Mat());
    });

    // 处理线程
    std::thread processor([&]() {
        while (true) {
            cv::Mat frame = raw_queue.pop();
            if (frame.empty()) {
                processed_queue.push(cv::Mat());
                break;
            }
            cv::Mat processed = processFrame(frame);
            processed_queue.push(processed);
        }
    });

    // 显示线程
    std::thread displayer([&]() {
        while (true) {
            cv::Mat frame = processed_queue.pop();
            if (frame.empty()) break;
            cv::imshow("Pipeline", frame);
            if (cv::waitKey(1) == 27) {
                stop = true;
                break;
            }
        }
    });

    reader.join();
    processor.join();
    displayer.join();
    return 0;
}
