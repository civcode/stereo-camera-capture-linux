#include "camera/stereo_camera.hpp"

#include <chrono>

#include "tic_toc_timer.h"

StereoCamera::StereoCamera(int camera_id_left, int camera_id_right, int width, int height, int fps, int buffer_count, int dt_max)
: id_left_(camera_id_left),
  id_right_(camera_id_right),
  width_(width),
  height_(height),
  fps_(fps),
  dt_max_(dt_max),
  frame_count_(0),
  cam_left_(camera_id_left, width, height, fps, buffer_count),
  cam_right_(camera_id_right, width, height, fps, buffer_count)
{
  init();
}
StereoCamera::~StereoCamera() {
  stop();
}
void StereoCamera::init() {

}

void StereoCamera::start() {
  cam_left_.start();
  cam_right_.start();
}

void StereoCamera::stop() {
  cam_left_.stop();
  cam_right_.stop();
}

bool StereoCamera::ready() const {
  return cam_left_.ready() && cam_right_.ready();
}

std::pair<std::shared_ptr<cv::Mat>, std::shared_ptr<cv::Mat>> StereoCamera::getLatestFrames() {
  while (!ready()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  bool is_synchronous;
  TicTocTimer timer;
  timer.tic();
  MultiBufferedCamera::TimestampedFrame& left_frame = cam_left_.getLatestFrame();
  std::cout << "Left frame capture time: " << timer.toc().ms().value<float>() << " ms" << std::endl;
  timer.tic();
  MultiBufferedCamera::TimestampedFrame& right_frame = cam_right_.getLatestFrame();
  std::cout << "Right frame capture time: " << timer.toc().ms().value<float>() << " ms" << std::endl;

  int count = 0;
  int step_r = 1;
  int step_l = 1;
  do {
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(left_frame.timestamp - right_frame.timestamp).count();
    if (std::abs(dt) < dt_max_) {
      // Frames are close enough
      // std::cout << "Frames are close enough, dt = " << dt << std::endl;
      std::cout << left_frame.used << " " << right_frame.used << std::endl;
      break;
    } else {
      if (dt > 0) {
        if (step_l >= 1)
          std::cout << "step_l = " << step_l << " @ frame " << frame_count_ << " @ dt = " << dt << std::endl;
        left_frame = cam_left_.getLatestFrameMinusN(step_l);
        step_l++;
      } else {
        if (step_r >= 1)
          std::cout << "step_r = " << step_r << " @ frame " << frame_count_ << " @ dt = "<< dt << std::endl;
        right_frame = cam_right_.getLatestFrameMinusN(step_r);
        step_r++;
      }
      if (left_frame.used || right_frame.used) {
        // std::cout << "Frame already used, exiting" << std::endl;
        break;
      }
      if (step_l >= cam_left_.getBufferCount() || step_r >= cam_right_.getBufferCount()) {
        std::cout << "step_l = " << step_l << ", step_r = " << step_r << std::endl;
        std::cout << "Buffer count reached, exiting" << std::endl;
        // exit(0);
        break;
      }
    }
  } while (true);

  frame_count_++;
  left_frame.used = true;
  right_frame.used = true;
  return std::pair<std::shared_ptr<cv::Mat>, std::shared_ptr<cv::Mat>> (
    std::make_shared<cv::Mat>(left_frame.frame),
    std::make_shared<cv::Mat>(right_frame.frame)
  );
}