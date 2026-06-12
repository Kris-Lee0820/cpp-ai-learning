#include "video_processor.h"

int main() {
    try {
	VideoProcessor processor("test.avi", 4);
	while (processor.readAndSubmit()) {
	    cv::Mat result;
	    if (processor.getProcessedFrame(result)) {
		cv::imshow("processed", result);
		if (cv::waitKey(30) == 27) break;
	    }
	}
    } catch (const std::exception& e) {
	std::cerr << e.what() << std::endl;
    }
}
