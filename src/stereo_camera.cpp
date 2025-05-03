#include "camera/stereo_camera.hpp"

#include <chrono>

StereoCamera::StereoCamera(int camera_id_left, int camera_id_right, int width, int height, int fps, int buffer_count)
: id_left_(camera_id_left),
  id_right_(camera_id_right),
  width_(width),
  height_(height),
  fps_(fps),
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
std::pair<cv::Mat, cv::Mat> StereoCamera::getLatestFrames() {
  while (!ready()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  bool is_synchronous;
  auto left_frame = cam_left_.getLatestFrame();
  auto right_frame = cam_right_.getLatestFrame();
  // while (true) {
  //   // is_synchronous = false;
  //   auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(left_frame.timestamp - right_frame.timestamp).count();
  //   if (std::abs(dt) < 10) {
  //     std::cout << "Synchronous frames captured" << std::endl;
  //     break;
  //   } else {
  //     std::cout << "Frames not synchronous, dt: " << dt << " ms" << std::endl;
  //     auto left_frame = cam_left_.getLatestFrame();
  //     auto right_frame = cam_right_.getLatestFrame();
  //   }
  // }
  int count = 0;
  do {
    // is_synchronous = false;
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(left_frame.timestamp - right_frame.timestamp).count();
    if (std::abs(dt) < 10) {
      // is_synchronous = true;
      // std::cout << "Synchronous frames captured" << std::endl;
      break;
    } else {
      std::cout << "Frames not synchronous, dt: " << dt << " ms," << " count " << count++ << std::endl;
      // std::this_thread::sleep_for(std::chrono::milliseconds(1));
      if (left_frame.timestamp < right_frame.timestamp) {
        // left_frame = cam_left_.getLatestFrame();
        right_frame = cam_right_.getLatestFrameMinusOne();
      } else {
        // right_frame = cam_right_.getLatestFrame();
        left_frame = cam_left_.getLatestFrameMinusOne();
      }
    }
  // } while (!is_synchronous);
  } while (true);

  return {left_frame.frame, right_frame.frame};
}