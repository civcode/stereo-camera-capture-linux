#include <opencv2/opencv.hpp>
#include <thread>

#include <pthread.h>
#include <sched.h>

#include "tictoc/timer.hpp"

bool running = true;
cv::Mat frame;

void capture() {
  cv::VideoCapture cap(0, cv::CAP_V4L2);
  if (!cap.isOpened()) {
    std::cerr << "Error: Could not open camera." << std::endl;
    return;
  }

  TicToc timer;
  while (running) {
    timer.tic();
    cap >> frame;
    std::cout << "Capture time: " << timer.toc().ms().value<float>() << " ms" << std::endl;
    if (frame.empty()) {
      std::cerr << "Error: No frame captured." << std::endl;
      continue;
    }
  }
}

void gui() {
  cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);
  while (running) {
    if (frame.empty()) {
      std::cerr << "Error: No frame captured." << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    cv::imshow("Camera", frame);
    if (cv::waitKey(1) >= 0) {
      running = false;
      break;
    }
  }
}

int main(int argc, char* argv[]) {

  std::thread capture_thread(capture);
  std::thread gui_thread(gui);

  while (running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

  };
  capture_thread.join();
  gui_thread.join();

  return 0;
}