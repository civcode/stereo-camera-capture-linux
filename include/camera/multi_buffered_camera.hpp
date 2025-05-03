#ifndef MULTI_BUFFERED_CAMERA_HPP_
#define MULTI_BUFFERED_CAMERA_HPP_

#include <opencv2/opencv.hpp>
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

class MultiBufferedCamera {
  public:
  struct TimestampedFrame {
    cv::Mat frame;
    std::chrono::steady_clock::time_point timestamp;
  };

  explicit MultiBufferedCamera(int camera_id, int width=640, int height=480, int fps=30, int buffer_count=10);
  ~MultiBufferedCamera();

  void start();
  void stop();
  bool ready() const;
  TimestampedFrame getLatestFrame();
  TimestampedFrame getLatestFrameMinusOne();
  TimestampedFrame getLatestFrameMinusN(int n);

  private:
  void init();
  void readyCamera();
  void captureLoop();

  int id_;
  int width_;
  int height_;
  int fps_;
  int buffer_count_;

  std::vector<TimestampedFrame> buffers_;
  std::atomic<int> write_index_;
  std::atomic<bool> running_;
  std::atomic<bool> cap_ready_;
  std::atomic<bool> stopped_;

  cv::VideoCapture cap_;
  std::thread capture_thread_;
  int count_;
};

#endif // MULTI_BUFFERED_CAMERA_HPP_