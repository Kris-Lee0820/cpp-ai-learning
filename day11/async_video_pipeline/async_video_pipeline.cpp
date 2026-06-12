#include <opencv2/opencv.hpp>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>
#include <chrono>
#include <thread>

class VideoPipeline {
public:
    VideoPipeline(const std::string& filename, int max_queue_size = 30)
   	: cap(filename), max_queue(max_queue_size), stop_flag(false) {
	if (!cap.isOpened()) throw std::runtime_error("Cannot open video");
    }

    void start() {
	reader = std::async(std::launch::async, &VideoPipeline::readFrames, this);
	processor = std::async(std::launch::async, &VideoPipeline::processFrames, this);
        writer = std::async(std::launch::async, &VideoPipeline::writeFrames, this);
    }

    void stop() {
	stop_flag = true;
	raw_cv.notify_all();
	proc_cv.notify_all();
	if (reader.valid()) reader.wait();
	if (processor.valid()) processor.wait();
        if (writer.valid()) writer.wait();
    }
private:
    cv::VideoCapture cap;
    int max_queue;
    std::atomic<bool> stop_flag;

    struct Frame {
	int idx;
	cv::Mat img;
    };
    std::queue<Frame> raw_queue, processed_queue;
    std::mutex raw_mutex, proc_mutex;
    std::condition_variable raw_cv, proc_cv;
    std::future<void> reader, processor, writer;

    void readFrames() {
	int idx = 0;
	while (!stop_flag) {
	    cv::Mat frame;
	    cap >> frame;
	    if (frame.empty()) break;
	    {
		std::unique_lock<std::mutex> lock(raw_mutex);
		raw_cv.wait(lock, [this] { return raw_queue.size() < max_queue || stop_flag; });
		if (stop_flag) break;
		raw_queue.push({idx++, frame});
	    }
	    raw_cv.notify_one();
	}
	// 发送结束信号
	{
	    std::lock_guard<std::mutex> lock(raw_mutex);
	    raw_queue.push({-1, cv::Mat()});
	}
	raw_cv.notify_one();
    }

    void processFrames() {
	while (true) {
	    Frame f;
	    {
		std::unique_lock<std::mutex> lock(raw_mutex);
		raw_cv.wait(lock, [this] { return !raw_queue.empty() || stop_flag; });
		if (stop_flag && raw_queue.empty()) break;
		f = raw_queue.front();
		raw_queue.pop();
		raw_cv.notify_one();
	    }
	    if (f.idx == -1) {
		// 传播结束
		std::lock_guard<std::mutex> lock(proc_mutex);
		processed_queue.push({-1, cv::Mat()});
		proc_cv.notify_one();
		break;
	    }
	    // 模拟耗时处理：CLAHE
	    cv::Mat lab, result;
	    cv::cvtColor(f.img, lab, cv::COLOR_BGR2Lab);
	    std::vector<cv::Mat> channels;
	    cv::split(lab, channels);
	    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
	    clahe->apply(channels[0], channels[0]);
	    cv::merge(channels, lab);
	    cv::cvtColor(lab, result, cv::COLOR_Lab2BGR);
	    {
		std::lock_guard<std::mutex> lock(proc_mutex);
		processed_queue.push({f.idx, result});
	    }
	    proc_cv.notify_one();
	}
    }

    void writeFrames() {
	int expected_idx = 0;
	while (true) {
	    Frame f;
	    {
		std::unique_lock<std::mutex> lock(proc_mutex);
		proc_cv.wait(lock, [this] { return !processed_queue.empty() || stop_flag; });
		if (stop_flag && processed_queue.empty()) break;
		f = processed_queue.front();
		processed_queue.pop();
	    }
	    if (f.idx == -1) break;
	    // 确保顺序输出（若乱序可缓存，此处简化）
	    cv::imshow("Pipeline", f.img);
            if (cv::waitKey(1) == 27) stop_flag = true;
	}
    }
};

int main() {
    VideoPipeline pipeline("test.mp4", 10);
    pipeline.start();
    std::this_thread::sleep_for(std::chrono::seconds(60)); // 运行60秒
    pipeline.stop();
    return 0;
}
