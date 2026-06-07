#ifndef IMAGE_PROCESSOR_ADVANCED_HPP
#define IMAGE_PROCESSOR_ADVANCED_HPP

#include <opencv2/opencv.hpp>
#include <functional>
#include <vector>
#include <memory>
#include <stdexcept>

// ==================== 类型特征 ====================
template<typename T> struct OpenCVDepth;
template<> struct OpenCVDepth<unsigned char> { static constexpr int value = CV_8U; };
template<> struct OpenCVDepth<float> { static constexpr int value = CV_32F; };

// 辅助函数：确保图像为 8UC3 格式，返回 cv::Mat（无论输入是 Mat 还是 UMat）
template<typename ImageType>
cv::Mat ensure8UC3(const ImageType& img) {
    // 获取 cv::Mat 对象（如果是 UMat 则调用 getMat，否则直接使用）
    cv::Mat mat;
    if constexpr (std::is_same_v<ImageType, cv::UMat>) {
        mat = img.getMat();
    } else {
        mat = img;
    }
    if (mat.depth() == CV_8U) {
        return mat;
    } else if (mat.depth() == CV_32F) {
        cv::Mat u8;
        mat.convertTo(u8, CV_8U, 255.0);
        return u8;
    } else {
        throw std::runtime_error("Unsupported depth");
    }
}

template<typename ImageType>
ImageType convertBack(const cv::Mat& result8U, int originalDepth) {
    if (originalDepth == CV_8U) {
        return result8U;
    } else {
        ImageType result;
        result8U.convertTo(result, CV_32F, 1.0/255.0);
        return result;
    }
}

// ==================== 基础图像处理器（支持 Mat / UMat） ====================
template<typename T, bool UseUMat = false>
class ImageProcessorLazy {
public:
    using ImageType = typename std::conditional<UseUMat, cv::UMat, cv::Mat>::type;

    explicit ImageProcessorLazy(const ImageType& img) : m_original(img.clone()) {}

    ImageProcessorLazy& addOperation(std::function<ImageType(const ImageType&)> op) {
        m_ops.push_back(op);
        return *this;
    }

    ImageType execute() const {
        ImageType result = m_original.clone();
        for (const auto& op : m_ops) {
            result = op(result);
        }
        return result;
    }

    void show(const std::string& win, int delay = 0) const {
        auto result = execute();
        cv::imshow(win, result);
        cv::waitKey(delay);
    }

    void save(const std::string& file) const {
        auto result = execute();
        cv::imwrite(file, result);
    }

private:
    ImageType m_original;
    std::vector<std::function<ImageType(const ImageType&)>> m_ops;
};

// ==================== 操作工厂（支持 float 自动转换） ====================
namespace ImageOps {

// 辅助：将原始图像转换为 8UC3（若需要），应用操作，再转换回原深度
template<typename T, bool UseUMat>
auto with8UConversion(std::function<cv::Mat(const cv::Mat&)> op8U) {
    return [op8U](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
        int originalDepth = img.depth();
        cv::Mat u8 = ensure8UC3(img);
        cv::Mat result8U = op8U(u8);
        return convertBack<typename ImageProcessorLazy<T, UseUMat>::ImageType>(result8U, originalDepth);
    };
}

// 1. 转为灰度（保持不变，灰度后仍为三通道）
template<typename T, bool UseUMat>
auto toGray() {
    return [](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
        typename ImageProcessorLazy<T, UseUMat>::ImageType gray, grayBGR;
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        cv::cvtColor(gray, grayBGR, cv::COLOR_GRAY2BGR);
        return grayBGR;
    };
}

// 2. 直方图均衡化（支持 float）
template<typename T, bool UseUMat>
auto equalizeHist() {
    return with8UConversion<T, UseUMat>([](const cv::Mat& u8img) {
        cv::Mat gray, equalized, result;
        cv::cvtColor(u8img, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, equalized);
        cv::cvtColor(equalized, result, cv::COLOR_GRAY2BGR);
        return result;
    });
}

// 3. CLAHE（自适应直方图均衡化）
template<typename T, bool UseUMat>
auto clahe(double clipLimit = 2.0, cv::Size tileGridSize = cv::Size(8,8)) {
    return with8UConversion<T, UseUMat>([clipLimit, tileGridSize](const cv::Mat& u8img) {
        cv::Mat gray, lab, result;
        cv::cvtColor(u8img, lab, cv::COLOR_BGR2Lab);
        std::vector<cv::Mat> channels;
        cv::split(lab, channels);
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileGridSize);
        clahe->apply(channels[0], channels[0]);
        cv::merge(channels, lab);
        cv::cvtColor(lab, result, cv::COLOR_Lab2BGR);
        return result;
    });
}

// 4. 形态学操作（腐蚀、膨胀等）同样需要支持 float
template<typename T, bool UseUMat>
auto erode(int kernelSize = 3) {
    return with8UConversion<T, UseUMat>([kernelSize](const cv::Mat& u8img) {
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
        cv::Mat eroded;
        cv::erode(u8img, eroded, kernel);
        return eroded;
    });
}

template<typename T, bool UseUMat>
auto dilate(int kernelSize = 3) {
    return with8UConversion<T, UseUMat>([kernelSize](const cv::Mat& u8img) {
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
        cv::Mat dilated;
        cv::dilate(u8img, dilated, kernel);
        return dilated;
    });
}

template<typename T, bool UseUMat>
auto morphologyOpen(int kernelSize = 3) {
    return with8UConversion<T, UseUMat>([kernelSize](const cv::Mat& u8img) {
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
        cv::Mat opened;
        cv::morphologyEx(u8img, opened, cv::MORPH_OPEN, kernel);
        return opened;
    });
}

template<typename T, bool UseUMat>
auto morphologyClose(int kernelSize = 3) {
    return with8UConversion<T, UseUMat>([kernelSize](const cv::Mat& u8img) {
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
        cv::Mat closed;
        cv::morphologyEx(u8img, closed, cv::MORPH_CLOSE, kernel);
        return closed;
    });
}

template<typename T, bool UseUMat>
auto gaussianBlur(int ksize = 5) {
    return [ksize](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
        typename ImageProcessorLazy<T, UseUMat>::ImageType blurred;
        cv::GaussianBlur(img, blurred, cv::Size(ksize, ksize), 0);
        return blurred;
    };
}

template<typename T, bool UseUMat>
auto canny(int low, int high) {
    return with8UConversion<T, UseUMat>([low, high](const cv::Mat& u8img) {
        cv::Mat gray, edges, edgesBGR;
        cv::cvtColor(u8img, gray, cv::COLOR_BGR2GRAY);
        cv::Canny(gray, edges, low, high);
        cv::cvtColor(edges, edgesBGR, cv::COLOR_GRAY2BGR);
        return edgesBGR;
    });
}

template<typename T, bool UseUMat>
auto resize(int width, int height) {
    return [width, height](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
        typename ImageProcessorLazy<T, UseUMat>::ImageType resized;
        cv::resize(img, resized, cv::Size(width, height));
        return resized;
    };
}

// ==================== 条件操作 ====================
// 计算图像平均亮度（0~255）
template<typename T, bool UseUMat>
double meanBrightness(const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    cv::Scalar mean = cv::mean(gray);
    return mean[0];
}

// ifThenElse: 如果条件成立，则应用 op，否则返回原图
template<typename T, bool UseUMat>
auto ifThenElse(std::function<bool(const typename ImageProcessorLazy<T, UseUMat>::ImageType&)> condition,
                std::function<typename ImageProcessorLazy<T, UseUMat>::ImageType(const typename ImageProcessorLazy<T, UseUMat>::ImageType&)> op) {
    return [condition, op](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) -> typename ImageProcessorLazy<T, UseUMat>::ImageType {
        if (condition(img)) {
            return op(img);
        } else {
            return img.clone();
        }
    };
}

} // namespace ImageOps

#endif // IMAGE_PROCESSOR_ADVANCED_HPP
