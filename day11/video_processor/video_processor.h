#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>

// 线程安全队列
template<typename T>
class SafeQueue {
    std::queue<T> queue;
    mutable std::mutex mtx;
    std::condition_variable cond;
    bool stop = false;
public:
    void push(T value) {
	std::lock_guard<std::mutex> lock(mtx);
	queue.push(std::move(value));
	cond.notify_one();
    }
    bool pop(T& value, bool wait = true) {
	std::unique_lock<std::mutex> lock(mtx);
	if (wait) {
	    cond.wait(lock, [this] { return !queue.empty() || stop; });
	    if (stop && queue.empty()) return false;
	} else {
	    if (queue.empty()) return false;
	}
	value = std::move(queue.front());
	queue.pop();
	return true;
    }
    void stopQueue() {
	std::lock_guard<std::mutex> lock(mtx);
	stop = true;
	cond.notify_all();
    }
    size_t size() const {
	std::lock_guard<std::mutex> lock(mtx);
	return queue.size();
    }
};

// 线程池（简化版，只支持无返回值任务，或使用future）
class ThreadPoolSimple {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
public:
    ThreadPoolSimple(size_t threads) : stop(false) {
	for (size_t i = 0; i < threads; ++i) {
	    workers.emplace_back([this] {
		while (true) {
		    std::function<void()> task;
		    {
			std::unique_lock<std::mutex> lock(queue_mutex);
			condition.wait(lock, [this] { return stop || !tasks.empty(); });
			if (stop && tasks.empty()) return;
			task = std::move(tasks.front());
			tasks.pop();
		    }
		    task();
		}		
	    });
	}
    }

    template<typename F>
    void enqueue(F &&f) {
	{
	    std::unique_lock<std::mutex> lock(queue_mutex);
	    if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
	    tasks.emplace(std::forward<F>(f));
	}
	condition.notify_one();
    }

    ~ThreadPoolSimple() {
	{
	    std::unique_lock<std::mutex> lock(queue_mutex);
	    stop = true;
	}
	condition.notify_all();
	for (auto &worker : workers) worker.join();
    }
};

// 视频处理器
class VideoProcessor {
    cv::VideoCapture cap;
    std::atomic<bool> stopFlag;
    SafeQueue<cv::Mat> inputQueue;
    SafeQueue<cv::Mat> outputQueue;
    ThreadPoolSimple pool;	// 这里pool其实没直接用于处理，因为处理循环是单线程的，但我们可以改为用pool处理每个帧块
    std::thread processThread;

    void processLoop() {
	while (!stopFlag) {
	    cv::Mat frame;
	    if (!inputQueue.pop(frame, true)) break;	// 队列停止且空则退出	    
	    // 执行图像处理（例如高斯模糊）
	    cv::Mat processed;
	    // 可以在这里应用任何OpenCV算法
	    cv::GaussianBlur(frame, processed, cv::Size(15, 15), 0);
	    // 或者用线程池并行处理每个块？这里直接处理整帧，线程池只用于不同帧的并行
	    outputQueue.push(processed);
	}
    }
public:
    VideoProcessor(const std::string& videoPath, int num_workers = 4) : stopFlag(false), pool(num_workers) {
	if (!cap.open(videoPath)) {
	    throw std::runtime_error("Cannot open video");
	}
	// 启动处理线程（从输入队列取帧，处理，放入输出队列）
	processThread = std::thread(&VideoProcessor::processLoop, this);
    }

    ~VideoProcessor() {
	stop();
	if (processThread.joinable()) processThread.join();
    }

    // 主线程调用：读取下一帧并交给处理队列
    bool readAndSubmit() {
	cv::Mat frame;
	if (!cap.read(frame)) return false;
	inputQueue.push(frame);
	return true;
    }

    // 主线程调用：获取已处理的一帧（阻塞直到有结果）
    bool getProcessedFrame(cv::Mat& outFrame) {
	return outputQueue.pop(outFrame);
    }

    void stop() {
	stopFlag = true;
	inputQueue.stopQueue();
	outputQueue.stopQueue();
    }
};
