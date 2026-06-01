Day 8 (2026-06-01) 学习日记：OpenCV DNN 人脸检测 —— SSD 成功，YuNet 暂未成功
📌 今日学习目标
理解 OpenCV DNN 模块的基本用法。

下载并加载 SSD 人脸检测模型（Caffe 格式）。

将 DNN 检测集成到视频处理程序中，实现实时人脸框绘制。

对比 SSD 与 YuNet 两种模型的部署差异。

尝试性能优化（缩小输入图像）。

✅ 已完成的核心任务
成功使用 SSD (Caffe) 模型实现人脸检测，可以实时在视频中绘制绿色框并显示置信度。

实现了 DNN 检测与原有亮度/对比度/Canny 功能的共存。

添加了键盘按键 m 在 Haar 和 DNN 之间切换（可选）。

研究了 DNN 速度优化方法（缩小检测图像尺寸，降低计算负担）。

📦 使用的模型
SSD Caffe 模型：

deploy.prototxt（网络结构）

res10_300x300_ssd_iter_140000_fp16.caffemodel（权重）

输入尺寸：300x300，输出：检测框 + 置信度。

🔧 实现步骤回顾
1. 下载模型文件
bash
cd ~/cpp-ai-learning/day8_adv
wget https://raw.githubusercontent.com/opencv/opencv_3rdparty/refs/heads/master/res10_300x300_ssd_iter_140000_fp16.caffemodel
wget https://raw.githubusercontent.com/opencv/opencv/master/samples/dnn/face_detector/deploy.prototxt
2. 加载模型并编写检测函数
cpp
#include <opencv2/dnn.hpp>
cv::dnn::Net net = cv::dnn::readNetFromCaffe(protoPath, modelPath);
// 对每一帧
cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, cv::Size(300, 300), cv::Scalar(104.0, 177.0, 123.0));
net.setInput(blob);
cv::Mat detections = net.forward();
// 解析输出，置信度 > 0.5 则绘制矩形
3. 性能优化
在送入网络前将原始图像缩小 0.5 倍，检测后再将坐标映射回原图。这样 速度提升约 4 倍（从 180ms/帧降到 45ms/帧）。

4. 集成到主循环
保留亮度/对比度/Canny 滑动条，添加 m 键切换 Haar / DNN。

在画面上显示当前使用的模型名称和置信度。

❌ 遇到的问题与解决
问题 1：SSD 输出的置信度异常低（0 或 0.088）
现象：confidence 值几乎全是 0 或 0.088，无法通过正常阈值（0.5）。

尝试：

降低阈值到 0.01，仍然无框。

打印原始 7 个输出值，发现数值都很小，且没有明显的 0~1 置信度。

更换全精度模型（非 fp16）后问题依旧。

最终解决：发现是 blob 预处理参数错误。正确的均值应为 (104, 117, 123)，且 swapRB = false。修正后置信度恢复正常（0.6~0.9），成功画出框。

问题 2：YuNet ONNX 模型加载失败
现象：使用 cv::FaceDetectorYN::create 加载 face_detection_yunet_2023mar.onnx 时，运行时报错：

text
Layer with requested id=-1 not found
原因分析：OpenCV 4.6.0 的 DNN 模块对某些 ONNX 算子支持不完善，或模型版本与 OpenCV 不兼容。

解决状态：未解决。暂时搁置，使用 SSD 模型继续学习。

问题 3：DNN 检测速度慢
现象：每帧耗时约 180ms（1080p 视频）。

优化：

将输入图像缩小 0.5 倍后检测，耗时降至 45ms。

还可以隔帧检测（每 2 帧检测一次），平均耗时 22.5ms，几乎无感知卡顿。

📊 效果对比
模型	速度（1080p）	准确率	环境要求
Haar Cascade	10ms/帧	较低，易误检	无
SSD (Caffe)	45ms/帧（优化后）	高，漏检少	OpenCV 4.x
YuNet (ONNX)	未成功	更高	需要 OpenCV 4.8+ 或特定版本
🧠 今日心得
DNN 模型的部署需要仔细核对预处理参数（均值、缩放、尺寸），差一点就会导致输出异常。

当遇到模型不兼容时，可以尝试更换模型格式（Caffe vs ONNX）或升级 OpenCV。

性能优化是工程落地的关键，缩小输入尺寸是最简单有效的加速手段。

📝 下一步计划
升级 OpenCV 到 4.8+，重新尝试 YuNet（它的小脸检测能力更强）。

将 SSD 模型集成到最终项目中，并添加模型切换功能（Haar / SSD）。

学习更多 DNN 后端加速方法（OpenVINO、CUDA）。
