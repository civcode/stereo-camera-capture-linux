#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <pthread.h>
#include <sched.h>

#include "tic_toc_timer.h"

bool running = true;
cv::Mat frame;

std::mutex mtx;
std::condition_variable cvx;
bool hasReachedPoint = false;
std::atomic<bool> just_written;

void capture() {
  cv::VideoCapture cap(0, cv::CAP_V4L2);
  if (!cap.isOpened()) {
    std::cerr << "Error: Could not open camera." << std::endl;
    return;
  }

  cap.set(cv::CAP_PROP_FPS, 60);

  TicTocTimer timer;
  while (running) {
    timer.tic();
    {
      // std::unique_lock<std::mutex> lock(mtx);
      // mtx.lock();
      cap >> frame;
      just_written.store(true);
      // mtx.unlock();
    }
    std::cout << "Capture time: " << timer.toc().ms().value<float>() << " ms" << std::endl;
    // {
    //   std::lock_guard<std::mutex> lock(mtx);
    //   hasReachedPoint = true;
    // }
    // cvx.notify_one();
    if (frame.empty()) {
      std::cerr << "Error: No frame captured." << std::endl;
      continue;
    }
  }
}

void gui() {
  cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);
  cv::Mat img;
  while (running) {
    if (frame.empty()) {
      std::cerr << "Error: No frame captured." << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    {
      // std::unique_lock<std::mutex> lock(mtx);
      // cvx.wait(lock, [] { return hasReachedPoint; });
      // hasReachedPoint = false;
      // mtx.lock();
      // img = frame.clone();
      // mtx.unlock();
    }
    while (!just_written.load()) {
      // std::cout << "Waiting for frame..." << std::endl;
      // std::this_thread::yield();
      // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    just_written.store(false);
    cv::imshow("Camera", frame);
    if (cv::waitKey(10) >= 0) {
      running = false;
      break;
    }
  }
}

int main(int argc, char* argv[]) {

  just_written.store(false);
  std::thread capture_thread(capture);
  std::thread gui_thread(gui);

  while (running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

  };
  capture_thread.join();
  gui_thread.join();

  return 0;
}