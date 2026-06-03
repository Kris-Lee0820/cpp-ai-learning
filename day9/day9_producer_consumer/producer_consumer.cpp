#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::queue<cv::Mat> frame_queue;
std::mutex mtx;
std::condition_variable cv_producer, cv_consumer;
const int MAX_QUEUE_SIZE = 5;
bool stop = false;

void consumer() {
    cv::namedWindow("Consumer", cv::WINDOW_NORMAL);
    while (!stop) {
	std::unique_lock<std::mutex> lock(mtx);
	cv_consumer.wait(lock, []{return !frame_queue.empty() || stop; });
	if (stop && frame_queue.empty()) break;
	cv::Mat frame = frame_queue.front();
	frame_queue.pop();
	lock.unlock();
	cv_producer.notify_one(); //通知生产者可以继续放入
	
	cv::imshow("Consumer", frame);
	cv::waitKey(30);
    }
}

int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) return -1;

    std::thread consumer_th(consumer);

    while (true) {
	cv::Mat frame;
	cap >> frame;
	if (frame.empty()) break;

	std::unique_lock<std::mutex> lock(mtx);
	cv_producer.wait(lock,[]{ return frame_queue.size() < MAX_QUEUE_SIZE; });
	frame_queue.push(frame.clone());
	lock.unlock();
	cv_consumer.notify_one();

	if (cv::waitKey(1) == 27) break;
    }

    stop = true;
    cv_consumer.notify_all();
    consumer_th.join();
    return 0;
}
