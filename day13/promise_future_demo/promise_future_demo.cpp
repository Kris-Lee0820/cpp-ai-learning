#include <opencv2/opencv.hpp>
#include <future>
#include <iostream>

cv::Mat processImg(const cv::Mat& src) {
    cv::Mat result;
    cv::cvtColor(src, result, cv::COLOR_BGR2GRAY);
    return result;
}

int main() {
    cv::Mat img = cv::imread("test.jpg");
    std::promise<cv::Mat> promise;
    std::future<cv::Mat> future = promise.get_future();

    std::thread thread([&]() {
        cv::Mat processed = processImg(img);
        promise.set_value(processed);
    });

    cv::Mat result = future.get();
    cv::imshow("Result", result);
    cv::waitKey(0);
    thread.join();
    return 0;
}