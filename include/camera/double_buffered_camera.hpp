#ifndef DOUBLE_BUFFERED_CAMERA_HPP_
#define DOUBLE_BUFFERED_CAMERA_HPP_

#include <opencv2/opencv.hpp>
#include <atomic>
#include <chrono>
#include <thread>
#include <array>
#include <memory>

#include <iostream>
#include <cstdlib>


class DoubleBufferedCamera {
  public:

  struct TimestampedFrame {
    cv::Mat frame;
    std::chrono::steady_clock::time_point timestamp;
  };

  explicit DoubleBufferedCamera(int camera_id, int width=640, int height=480, int fps=30);
  ~DoubleBufferedCamera();

  void start();
  void stop();
  bool ready() const;
  TimestampedFrame getLatestFrame();

  private:
  cv::VideoCapture cap_;
  std::atomic<int> write_index_; // 0 or 1
  std::atomic<bool> running_;
  std::atomic<bool> cap_ready_;
  std::array<TimestampedFrame, 2> buffers_;
  std::thread capture_thread_;
  int count_;
  int id_;
  int width_;
  int height_;
  int fps_;

  void init();
  void readyCamera();
  void captureLoop();
};

#endif // DOUBLE_BUFFERED_CAMERA_HPP_