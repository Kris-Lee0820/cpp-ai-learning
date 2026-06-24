#include <opencv2/opencv.hpp>
#include <future>
#include <iostream>

cv::Mat readImg(const std::string& path) {
    cv::Mat img = cv::imread(path);
    if (img.empty()) throw std::runtime_error("Cannot read");
    return img;
}
cv::Mat grayscale(cv::Mat img) {
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    return gray;
}
cv::Mat resizeImg(cv::Mat img, int w, int h) {
    cv::Mat resized;
    cv::resize(img, resized, cv::Size(w, h));
    return resized;
}
void showImg(cv::Mat img) {
    cv::imshow("Chain Result", img);
    cv::waitKey(0);
}

int main() {
    // auto future = std::async(std::launch::async, readImg, "test.jpg")
    //     .then([](cv::Mat img) { return grayscale(img); })   // C++17 没有 .then，改用 std::async 嵌套
    //     .then([](cv::Mat img) { return resizeImg(img, 400,300); })
    //     .then([](cv::Mat img) { showImg(img); return 0; });
    // 实际需手动链式，下面给出正确写法（使用 then 扩展或显式等待）
    // 以下用显式等待：
    auto f1 = std::async(std::launch::async, readImg, "test.jpg");
    auto img1 = f1.get();
    auto f2 = std::async(std::launch::async, grayscale, img1);
    auto img2 = f2.get();
    auto f3 = std::async(std::launch::async, resizeImg, img2, 400, 300);
    auto img3 = f3.get();
    showImg(img3);
    return 0;
}