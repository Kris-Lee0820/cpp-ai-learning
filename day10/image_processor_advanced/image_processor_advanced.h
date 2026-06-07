#ifndef IMAGE_PROCESSOR_ADVANCED_H
#define IMAGE_PROCESSOR_ADVANCED_H

#include <opencv2/opencv.hpp>
#include <functional>
#include <vector>
#include <memory>
#include <stdexcept>

// ==================== 类型特征 ====================
template<typename T> struct OpenCVDepth;
template<> struct OpenCVDepth<unsigned char> { static constexpr int value = CV_8U; };
template<> struct OpenCVDepth<float> { static constexpr int value = CV_32F; };

// ==================== 辅助函数：确保图像为 8UC3（用于处理函数） ====================
template<typename ImageType>
cv::Mat ensure8UC3(const ImageType& img) {
    cv::Mat mat;
    if constexpr (std::is_same_v<ImageType, cv::UMat>) {
	mat = img.getMat();
    } else {
	mat = img;
    }

    if (img.depth() == CV_8U) {
	return mat;
    } else if (img.depth() == CV_32F) {
	cv::Mat u8;
	img.convertTo(u8, CV_8U, 255.0);
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
	cv::Mat result;
	result8U.convertTo(result, CV_32F, 1.0/255.0);
	return result;
    }
}

// ==================== 基础图像处理器（支持 Mat / UMat） ====================
template<typename T, bool UseUMat = false>
class ImageProcessorLazy{
public:
    using ImageType = typename std::conditional<UseUMat, cv::UMat, cv::Mat>::type;
    
    // 构造函数：接受图像，立即存储（非延迟）
    explicit ImageProcessorLazy(const ImageType& img) : m_original(img.clone()) {}

    // 添加一个操作（延迟记录）
    ImageProcessorLazy& addOperation(std::function<ImageType(const ImageType&)> op) {
	m_ops.push_back(op);
	return *this;
    }

    // 执行所有已记录的操作，返回最终图像
    ImageType execute() const {
	ImageType result = m_original.clone();
	for (const auto& op : m_ops) {
	    result = op(result);
	}
	return result;
    }

    // 便捷方法：直接执行并显示
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

// ==================== 预定义的操作工厂 ====================
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

// 1. 转为灰度图（输出仍为三通道，便于后续操作）
template<typename T, bool UseUMat>
auto toGray() {
    return [](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
	typename ImageProcessorLazy<T, UseUMat>::ImageType gray, grayBGR;
	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
	cv::cvtColor(gray, grayBGR, cv::COLOR_GRAY2BGR);
	return grayBGR;
    };
}

// 2. 直方图均衡化（要求输入为灰度图，这里内部处理）
template<typename T, bool UseUMat>
auto equalizeHist() {
    return [](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
	typename ImageProcessorLazy<T, UseUMat>::ImageType gray, equalized, result;
	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
	cv::equalizeHist(gray, equalized);
	cv::cvtColor(equalized, result, cv::COLOR_GRAY2BGR);
	return result;
    };
}

// 3. CLAHE（自适应直方图均衡化）
template<typename T, bool UseUMat>
auto clahe(double clipLimit = 2.0, cv::Size tileGridSize = cv::Size(8, 8)) {
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

// 3. 形态学操作：腐蚀
template<typename T, bool UseUMat>
auto erode(int kernelSize = 3) {
    return [kernelSize](const typename ImageProcessorLazy<T, UseUMat>::ImageType &img) {
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
	typename ImageProcessorLazy<T, UseUMat>::ImageType eroded;
	cv::erode(img, eroded, kernel);
	return eroded;
    };
}

// 4. 形态学操作：膨胀
template<typename T, bool UseUMat>
auto dilate(int kernelSize = 3) {
    return [kernelSize](const typename ImageProcessorLazy<T, UseUMat>::ImageType &img) {
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
	typename ImageProcessorLazy<T, UseUMat>::ImageType dilated;
	cv::dilate(img, dilated, kernel);
	return dilated;
    };
}

// 5. 开运算（先腐蚀后膨胀）
template<typename T, bool UseUMat>
auto morphologyOpen(int kernelSize = 3) {
    return [kernelSize](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
	typename ImageProcessorLazy<T, UseUMat>::ImageType opened;
	cv::morphologyEx(img, opened, cv::MORPH_OPEN, kernel);
	return opened;
    };
}

// 6. 闭运算（先膨胀后腐蚀）
template<typename T, bool UseUMat>
auto morphologyClose(int kernelSize = 3) {
    return [kernelSize](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
	typename ImageProcessorLazy<T, UseUMat>::ImageType closed;
	cv::morphologyEx(img, closed, cv::MORPH_CLOSE, kernel);
	return closed;
    };
}

// 7. 高斯模糊
template<typename T, bool UseUMat>
auto gaussianBlur(int kSize = 5) {
    return [kSize](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
	typename ImageProcessorLazy<T, UseUMat>::ImageType blurred;
	cv::GaussianBlur(img, blurred, cv::Size(kSize, kSize), 0);
	return blurred;
    };
}

// 8. Canny 边缘检测
template<typename T, bool UseUMat>
auto canny(int low, int high) {
    return [low, high](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
	typename ImageProcessorLazy<T, UseUMat>::ImageType gray, edges, edgesBGR;
	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
	cv::Canny(gray, edges, low, high);
	cv::cvtColor(edges, edgesBGR, cv::COLOR_GRAY2BGR);
	return edgesBGR;
    };
}

// 9. 缩放
template<typename T, bool UseUMat>
auto resize(int width, int height) {
    return [width, height](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
	typename ImageProcessorLazy<T, UseUMat>::ImageType resized;
	cv::resize(img, resized, cv::Size(width, height));
	return resized;
    };
}

// 10. 旋转
template<typename T, bool UseUMat>
auto rotate(double angle) {
    return [angle](const typename ImageProcessorLazy<T, UseUMat>::ImageType& img) {
	typename ImageProcessorLazy<T, UseUMat>::ImageType rotated;
	cv::Point2f center(img.cols/2.0, img.rows/2.0);
	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
	cv::warpAffine(img, rotated, rot, img.size());
	return rotated;
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
auto ifThenElse(std::function<bool(const typename ImageProcessorLazy<T, UseUMat>::ImageType&)> condition, std::function<typename ImageProcessorLazy<T, UseUMat>::ImageType(const typename ImageProcessorLazy<T, UseUMat>::ImageType&)> op) {
    return [condition, op](const typename ImageProcessorLazy<T, UseUMat>::ImageType &img) -> typename ImageProcessorLazy<T, UseUMat>::ImageType {
	if (condition(img)) {
	    return op(img);
	} else {
	    return img.clone();
	}
    };
}
} // namespace ImageOps

#endif // IMAGE_PROCESSOR_ADVANCED_H
