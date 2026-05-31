Day 7 (2026-05-31) 学习日记：人脸检测 + 眼睛检测 + 背景模糊
📌 今日目标
在视频处理程序中集成人脸检测（Haar Cascade）和眼睛检测。

添加滑动条实时调整人脸检测参数（scaleFactor、minNeighbors、minSize）。

实现背景模糊（马赛克效果）：仅人脸区域保持清晰，其余部分模糊。

显示当前所有参数数值在画面上。

🧠 技术要点
OpenCV 的 CascadeClassifier 加载 Haar 特征 XML 模型。

detectMultiScale 的参数含义与调优技巧。

图像 ROI 操作：将清晰人脸区域复制到模糊背景上。

滑动条回调函数中实时更新参数并刷新画面。

多线程安全：回调中直接调用 processFrame() 立即重绘。

📂 项目结构
text
day7_adv/
├── video_demo.cpp          # 完整源码
├── CMakeLists.txt
├── haarcascade_frontalface_default.xml
├── haarcascade_eye.xml
├── vtest.avi               # 测试视频
└── build/                  # 编译目录
🔧 实现的功能模块
亮度/对比度调整（之前已有）

Canny 边缘检测（之前已有）

人脸检测（绿色矩形）

眼睛检测（蓝色矩形，仅在人脸区域内）

背景模糊（高斯模糊，保留人脸清晰）

实时参数显示（FPS、亮度、对比度、Canny 阈值、人脸检测参数）

键盘控制：f 开关人脸检测，空格暂停，ESC 退出

🧪 核心代码片段
1. 加载级联分类器
cpp
cv::CascadeClassifier face_cascade, eye_cascade;
if (!face_cascade.load("haarcascade_frontalface_default.xml"))
    std::cerr << "人脸模型加载失败" << std::endl;
if (!eye_cascade.load("haarcascade_eye.xml"))
    std::cerr << "眼睛模型加载失败" << std::endl;
2. 人脸检测参数滑动条（关键修正）
cpp
int g_haar_scaleFactor_int = 105;  // 存储整数，实际 scaleFactor = int/100.0
cv::createTrackbar("Face Scale", "Video", &g_haar_scaleFactor_int, 200, onFaceScaleChange);
cv::setTrackbarMin("Face Scale", "Video", 101);  // 确保 scaleFactor > 1
回调函数中转换为浮点数并刷新：

cpp
void onFaceScaleChange(int, void*) {
    if (g_haar_scaleFactor_int < 101) g_haar_scaleFactor_int = 101;
    g_haar_scaleFactor = g_haar_scaleFactor_int / 100.0;
    processFrame();
}
3. 背景模糊（马赛克效果）
cpp
if (g_face_detection_enabled && !faces.empty() && g_edge_mode == 0) {
    cv::Mat blurred;
    cv::GaussianBlur(g_display_frame, blurred, cv::Size(31, 31), 0);
    g_display_frame = blurred.clone();
    for (const auto& face : faces) {
        cv::Rect roi = face & cv::Rect(0, 0, g_display_frame.cols, g_display_frame.rows);
        cv::Mat face_roi = adjusted(roi);  // 原始清晰图像
        face_roi.copyTo(g_display_frame(roi));
    }
}
4. 眼睛检测（在每个人脸 ROI 内）
cpp
cv::Mat face_roi_gray;
cv::cvtColor(g_display_frame(face), face_roi_gray, cv::COLOR_BGR2GRAY);
std::vector<cv::Rect> eyes;
eye_cascade.detectMultiScale(face_roi_gray, eyes, 1.05, 8);
for (const auto& eye : eyes) {
    cv::Rect eye_abs(face.x + eye.x, face.y + eye.y, eye.width, eye.height);
    cv::rectangle(g_display_frame, eye_abs, cv::Scalar(255, 0, 0), 2);
}
❌ 遇到的问题与解决方案
问题 1：detectMultiScale 崩溃，报错 scaleFactor > 1 断言失败
现象：运行程序时，滑动条拉到最左边（值为0）或 scaleFactor ≤ 1 时直接崩溃。

原因：OpenCV 要求 scaleFactor 严格大于 1，且图像深度必须为 CV_8U。

解决：

将滑动条范围设置为 101~200（对应 1.01~2.00），并设置最小值 cv::setTrackbarMin("Face Scale", "Video", 101);。

在回调函数中增加防呆：if (g_haar_scaleFactor_int < 101) g_haar_scaleFactor_int = 101;

确保输入灰度图是 CV_8U（cvtColor 默认输出 CV_8U，无需额外处理）。

问题 2：眼睛检测误检率高，或检测不到
现象：在人脸框内画出了很多蓝色矩形，但很多不是眼睛。

原因：眼睛模型对光照、姿态敏感，且人脸 ROI 内可能存在眉毛、眼镜框等干扰。

解决：

调整眼睛检测参数：detectMultiScale(face_roi_gray, eyes, 1.05, 8) 中 minNeighbors 从默认的 3 提高到 8，减少误检。

添加简单过滤：只保留眼睛大小合理的区域（比如宽度 > 人脸宽度的 1/5 则忽略）。

也可以限制检测区域为人脸的上半部分（face_roi = face_roi(cv::Rect(0,0,face_roi.cols, face_roi.rows/2));）。

问题 3：背景模糊后整个人脸区域也被模糊了（复制逻辑错误）
现象：整个画面都是模糊的，没有人脸清晰区域。

原因：在调用 GaussianBlur 后直接赋值给了 g_display_frame，但没有把清晰人脸区域复制回来，或者复制的源图像错误。

解决：确保先复制清晰的人脸区域到临时变量，然后模糊整个图像，最后再用 copyTo 将清晰区域贴回。注意要克隆图像，避免引用混乱。

问题 4：滑动条拖动不流畅，画面卡顿
原因：每帧都进行人脸检测 + 眼睛检测 + 背景模糊，计算量大。

优化：

降低人脸检测频率（例如每 3 帧检测一次）。

减小 minSize 和 maxSize 可以加快检测速度。

将 scaleFactor 适当调大（如 1.1）可减少金字塔层数。

背景模糊核大小 Size(31,31) 改为更小的值（如 (15,15)）以加快速度。

问题 5：按 f 开关人脸检测后，背景模糊未立即禁用
原因：processFrame() 中根据 g_face_detection_enabled 决定是否执行背景模糊和检测，但按键后只修改了变量，没有重新处理当前帧。

解决：在按键处理中调用 processFrame()：

cpp
if (key == 'f') {
    g_face_detection_enabled = !g_face_detection_enabled;
    processFrame();   // 立即刷新
}
🚀 最终运行效果
视频播放流畅（约 25-30 FPS，取决于参数）。

人脸被绿色矩形框出，眼睛用蓝色矩形标出。

背景呈高斯模糊效果，只有人脸区域清晰可见。

左上角实时显示所有参数值，拖动滑动条时立即更新。

按 f 可随时开关人脸检测，方便对比。

📸 运行时截图说明
（此处可粘贴终端输出或描述画面内容）

📖 学习心得
Haar Cascade 虽然古老，但在资源有限或需要快速实现时仍然有用。参数调优是关键，尤其是 minNeighbors 和 scaleFactor。

背景模糊（马赛克）是一个很实用的隐私保护技巧，原理简单但效果惊艳。

滑动条回调函数中调用 processFrame() 必须小心，避免死循环或重复进入。这里因为 processFrame() 不会修改滑动条的值，所以安全。

眼睛检测的准确性远低于人脸检测，后续可以考虑使用深度学习模型（如 YuNet）或关键点检测（如 dlib）来提升效果。

📝 后续改进方向
用 DNN 模型（YuNet、MTCNN）替代 Haar，提高检测精度和角度鲁棒性。

增加人脸关键点检测（眼睛、鼻子、嘴巴）并绘制。

实现人脸跟踪，减少每帧重复检测的计算量。

加入人脸识别（谁是谁）功能。

💾 代码仓库
所有源码已提交到 GitHub：

bash
cd ~/cpp-ai-learning
git add day7_adv/
git commit -m "Day7: face detection + eye detection + background blur with trackbars"
git push
🕒 学习时长
约 3 小时（编码 + 调试 + 文档）

日记撰写人：Kris
日期：2026-05-31
