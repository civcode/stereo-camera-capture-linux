#include <opencv2/opencv.hpp>
#include <thread>

#include "tic_toc_timer.h"

int main(int argc, char* argv[]) {
  // MultiBufferedCamera camera(0, 640, 480, 60);
  // MultiBufferedCamera camera(0, 320, 240, 187);
  // StereoCamera camera(0, 1, 640, 480, 15, 10);
  // StereoCamera camera(0, 1, 640, 480, 30, 10, 5);

  cv::VideoCapture cap(0, cv::CAP_V4L2);

  cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
  cap.set(cv::CAP_PROP_FPS, 30);
  // cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

  // system("v4l2-ctl --set-ctrl=video_bitrate=0"); // If using V4L2


  if (!cap.isOpened()) {
    std::cerr << "Error: Could not open camera." << std::endl;
    return -1;
  }

  cv::Mat frame;

  TicTocTimer timer;
  while (true) {
    timer.tic();
    cap >> frame;
    // timer.toc();
    // cap.grab();
    // cap.retrieve(frame);
    float dt = timer.toc().ms().value<float>();
    if (dt > 40) {
      std::cout << "dt: " << dt << " ms" << std::endl;
    }
    // std::cout << "dt: " << dt << " ms" << std::endl;
    if (frame.empty()) {
      std::cerr << "Error: No frame captured." << std::endl;
      continue;
      // break;
    }
    // std::this_thread::sleep_for(std::chrono::milliseconds(35));

    // cv::imshow("Camera", frame);
    // if (cv::waitKey(1) >= 0) break; // Exit on key press
  }

  cap.release();
  // cv::destroyAllWindows();

  return 0;
}