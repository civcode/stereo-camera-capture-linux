#include <filesystem>
#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "cxxopts.hpp"

#include "fmt/base.h"

#include "camera/mono.hpp"
#include "camera/stereo.hpp"

#include "tictoc/timer.hpp"

namespace fs = std::filesystem;

using std::cout;
using std::endl;


int main(int argc, char* argv[]) {

  // MonoCamera camera(0, 640, 480, 60);
  // MonoCamera camera(0, 320, 240, 187);
  // StereoCamera camera(0, 1, 640, 480, 15, 10);
  StereoCamera camera(0, 1, 640, 480, 30);
  // StereoCamera camera(0, 1, 320, 240, 187, 10, 3);
  camera.start();
  cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);
  cv::Mat frame[2];

  int count = 0;

  double fps = 0.0;
  int frameCount = 0;
  double tickFrequency = cv::getTickFrequency();
  int64 startTime = cv::getTickCount();
  TicToc timer;
  while (true) {
    // auto timestamped_frame = camera.getLatestFrames();
    timer.tic();
    auto images = camera.getLatestFrames();
    float dt = timer.toc().ms().value<float>();
    // cout << "dt in ms: " << dt << endl;
    // frame = timestamped_frame.frame;
    // frame = timestamped_frame.frame.clone();
    frame[0] = *images.first;
    frame[1] = *images.second;
    if (frame[0].empty()) {
      cout << "No frame captured" << endl;
      // std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    cv::Mat img;
    cv::hconcat(frame[0], frame[1], img);
    // cv::resize(img, img, cv::Size(320, 240));
    timer.tic();
    cv::imshow("Camera", img);
    // cv::imshow("Camera Left", frame[0]);
    // cv::imshow("Camera Right", frame[1]);
    if (cv::waitKey(1) == 27) break;
    // cout << "dt in ms: " << timer.toc().ms().value<float>() << endl;
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // cout << "Frame " << count++ << " captured" << endl;

    frameCount++;
    int64 currentTime = cv::getTickCount();
    double timePassed = (currentTime - startTime) / tickFrequency;

    if (timePassed >= 1.0) {
        fps = frameCount / timePassed;
        startTime = currentTime;
        frameCount = 0;
    }
    // printf("\rFPS: %.1f dt: %f", fps, dt);
    // printf("\rFPS: %.1f", fps);
    // cout << std::flush;
    fmt::print("\rFPS: {:.1f}", fps);
    std::cout << std::flush;
    // cout << "FPS: " << fps << std::endl;
    // std::this_thread::yield();
  }
  cout << endl;
  camera.stop();
  cv::destroyAllWindows();

  return 0;
}
