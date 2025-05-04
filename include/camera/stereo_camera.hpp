#ifndef STEREO_CAMERA_HPP_
#define STEREO_CAMERA_HPP_


#include <opencv2/opencv.hpp>
// #include <atomic>
#include <chrono>
#include <memory>
#include <iostream>
// #include <thread>
// #include <vector>
#include "camera/multi_buffered_camera.hpp"

class StereoCamera {
  public:
  explicit StereoCamera(int camera_id_left, int camera_id_right, int width=640, int height=480, int fps=30, int buffer_count=10);
  ~StereoCamera();
  void start();
  void stop();
  bool ready() const;
  // std::pair<cv::Mat&, cv::Mat&> getLatestFrames();
  std::pair<std::shared_ptr<cv::Mat>, std::shared_ptr<cv::Mat>> getLatestFrames();

  private:
  void init();
  void readyCamera();
  void captureLoop();
  int id_left_;
  int id_right_;
  int width_;
  int height_;
  int fps_;
  // cv::VideoCapture cap_left_;
  // cv::VideoCapture cap_right_;
  MultiBufferedCamera cam_left_;
  MultiBufferedCamera cam_right_;


};

#endif // STEREO_CAMERA_HPP_