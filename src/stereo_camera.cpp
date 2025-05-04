#include "camera/stereo_camera.hpp"

#include <chrono>

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
  auto left_frame = cam_left_.getLatestFrame();
  auto right_frame = cam_right_.getLatestFrame();
  int count = 0;
  int step_r = 1;
  int step_l = 1;
  do {
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(left_frame.timestamp - right_frame.timestamp).count();
    if (std::abs(dt) < dt_max_) {
      // Frames are close enough
      break;
    } else {
      if (left_frame.timestamp < right_frame.timestamp) {
        if (step_r > 1)
          std::cout << "step_r = " << step_r << " @ frame " << frame_count_ << " @ dt = " << dt << std::endl;
        right_frame = cam_right_.getLatestFrameMinusN(step_r++);
      } else {
        if (step_l > 1)
          std::cout << "step_l = " << step_l << " @ frame " << frame_count_ << " @ dt = "<< dt << std::endl;
        left_frame = cam_left_.getLatestFrameMinusN(step_l++);
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
  return std::pair<std::shared_ptr<cv::Mat>, std::shared_ptr<cv::Mat>> (
    std::make_shared<cv::Mat>(left_frame.frame),
    std::make_shared<cv::Mat>(right_frame.frame)
  );
}