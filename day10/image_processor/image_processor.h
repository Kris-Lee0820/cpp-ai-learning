#ifndef IMAGE_PROCESSOR_HPP
#define IMAGE_PROCESSOR_HPP

#include <opencv2/opencv.hpp>
#include <stdexcept>

template<typename T> struct OpenCVDataType;
template<> struct OpenCVDataType<unsigned char> { static constexpr int value = CV_8U; };
template<> struct OpenCVDataType<float> { static constexpr int value = CV_32F; };

// 通用模板声明
template<typename T>
class ImageProcessor {
public:
    explicit ImageProcessor(const cv::Mat& img) : m_image(img.clone()) {
        if (m_image.depth() != OpenCVDataType<T>::value) {
            throw std::runtime_error("Image type does not match template parameter T");
        }
    }
    cv::Mat getImage() const { return m_image; }

    // 通用版本（用于 unsigned char）
    ImageProcessor<T> toGray() const {
        cv::Mat gray, grayBGR;
        cv::cvtColor(m_image, gray, cv::COLOR_BGR2GRAY);
        cv::cvtColor(gray, grayBGR, cv::COLOR_GRAY2BGR);
        return ImageProcessor<T>(grayBGR);
    }

    ImageProcessor<T> resize(int w, int h) const {
        cv::Mat resized;
        cv::resize(m_image, resized, cv::Size(w, h));
        return ImageProcessor<T>(resized);
    }

    ImageProcessor<T> gaussianBlur(int ksize) const {
        cv::Mat blurred;
        cv::GaussianBlur(m_image, blurred, cv::Size(ksize, ksize), 0);
        return ImageProcessor<T>(blurred);
    }

    // 通用 Canny（要求输入为 8U，否则特化处理）
    ImageProcessor<T> canny(int low, int high) const {
        static_assert(std::is_same<T, unsigned char>::value, "Canny only works for unsigned char, please use specialization for float");
        cv::Mat gray, edges, edgesBGR;
        cv::cvtColor(m_image, gray, cv::COLOR_BGR2GRAY);
        cv::Canny(gray, edges, low, high);
        cv::cvtColor(edges, edgesBGR, cv::COLOR_GRAY2BGR);
        return ImageProcessor<T>(edgesBGR);
    }

    void show(const std::string& win, int delay=0) const {
        cv::imshow(win, m_image);
        cv::waitKey(delay);
    }
    void save(const std::string& file) const { cv::imwrite(file, m_image); }

protected:
    cv::Mat m_image;
};

// 特化：针对 float 图像的 toGray
template<>
ImageProcessor<float> ImageProcessor<float>::toGray() const {
    cv::Mat uchar_img;
    m_image.convertTo(uchar_img, CV_8U, 255.0);
    cv::Mat gray;
    cv::cvtColor(uchar_img, gray, cv::COLOR_BGR2GRAY);
    cv::Mat grayBGR;
    cv::cvtColor(gray, grayBGR, cv::COLOR_GRAY2BGR);
    cv::Mat float_result;
    grayBGR.convertTo(float_result, CV_32F, 1.0/255.0);
    return ImageProcessor<float>(float_result);
}

// 特化：针对 float 图像的 canny（先转 8U，Canny，再转回 float）
template<>
ImageProcessor<float> ImageProcessor<float>::canny(int low, int high) const {
    // 转为 8U 范围 0-255
    cv::Mat uchar_img;
    m_image.convertTo(uchar_img, CV_8U, 255.0);
    // 灰度化
    cv::Mat gray;
    cv::cvtColor(uchar_img, gray, cv::COLOR_BGR2GRAY);
    // Canny
    cv::Mat edges;
    cv::Canny(gray, edges, low, high);
    // 转回三通道 BGR 便于显示
    cv::Mat edgesBGR;
    cv::cvtColor(edges, edgesBGR, cv::COLOR_GRAY2BGR);
    // 转回 float 范围 0.0-1.0
    cv::Mat float_edges;
    edgesBGR.convertTo(float_edges, CV_32F, 1.0/255.0);
    return ImageProcessor<float>(float_edges);
}

// 辅助函数
template<typename T>
ImageProcessor<T> makeImageProcessor(const cv::Mat& img) {
    return ImageProcessor<T>(img);
}

#endif
