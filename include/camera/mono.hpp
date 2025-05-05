#ifndef MULTI_BUFFERED_CAMERA_HPP_
#define MULTI_BUFFERED_CAMERA_HPP_

#include <opencv2/opencv.hpp>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

class MonoCamera {
  public:
  struct TimestampedFrame {
    bool used;
    cv::Mat frame;
    std::chrono::steady_clock::time_point timestamp;
  };

  explicit MonoCamera(int camera_id, int width=640, int height=480, int fps=30, int buffer_count=10);
  ~MonoCamera();

  void start();
  void stop();
  bool ready() const;
  int getBufferCount() const;
  TimestampedFrame& getLatestFrame();
  TimestampedFrame& getLatestFrameMinusOne();
  TimestampedFrame& getLatestFrameMinusN(int n);

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
  std::atomic<int> last_read_index_;
  std::atomic<int> cap_count_;
  std::atomic<bool> just_written_;

  std::mutex mtx_;
  std::condition_variable cv_;
  bool wait_for_new_frame_;

  cv::VideoCapture cap_;
  std::thread capture_thread_;
  int count_;
};

#endif // MULTI_BUFFERED_CAMERA_HPP_