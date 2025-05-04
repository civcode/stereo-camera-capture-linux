#include <filesystem>
#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "cxxopts.hpp"

#include "camera/double_buffered_camera.hpp"
#include "camera/multi_buffered_camera.hpp"
#include "camera/ps3_eye.hpp"
#include "camera/stereo_camera.hpp"

namespace fs = std::filesystem;

using std::cout;
using std::endl;


int main(int argc, char* argv[]) {

  // MultiBufferedCamera camera(0, 640, 480, 60);
  // MultiBufferedCamera camera(0, 320, 240, 187);
  // StereoCamera camera(0, 1, 640, 480, 15, 10);
  StereoCamera camera(0, 1, 640, 480, 60, 10);
  // StereoCamera camera(0, 1, 320, 240, 90, 10);
  camera.start();
  cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);
  cv::Mat frame[2];

  int count = 0;
  while (true) {
    // auto timestamped_frame = camera.getLatestFrames();
    auto images = camera.getLatestFrames();
    // frame = timestamped_frame.frame;
    // frame = timestamped_frame.frame.clone();
    frame[0] = *images.first;
    frame[1] = *images.second;
    if (frame[0].empty()) {
      cout << "No frame captured" << endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    cv::Mat img;
    cv::hconcat(frame[0], frame[1], img);
    cv::imshow("Camera", img);
    // cv::imshow("Camera Left", frame[0]);
    // cv::imshow("Camera Right", frame[1]);
    if (cv::waitKey(1) == 27) break;
    // cout << "Frame " << count++ << " captured" << endl;
  }
  camera.stop();
  cv::destroyAllWindows();

  return 0;
}