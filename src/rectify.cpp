#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "cxxopts.hpp"

#include "camera/stereo_camera.hpp"
#include "tic_toc_timer.h"

using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
  cv::Mat R1, R2, P1, P2, Q;
  cv::Mat K1, K2, R;
  cv::Vec3d T;
  cv::Mat D1, D2;
  cv::Mat img1;
  cv::Mat img2;

  std::string calib_file = "/home/chris/workspace/opencv/stereo-calibration/build/stereo_calib.json";

  cv::FileStorage fs1(calib_file, cv::FileStorage::READ);
  if (!fs1.isOpened()) {
    cout << "Error opening calibration file: " << calib_file << endl;
    return 1;
  }

  fs1["K1"] >> K1;
  fs1["K2"] >> K2;
  fs1["D1"] >> D1;
  fs1["D2"] >> D2;
  fs1["R"] >> R;
  fs1["T"] >> T;
  fs1["R1"] >> R1;
  fs1["R2"] >> R2;
  fs1["P1"] >> P1;
  fs1["P2"] >> P2;
  fs1["Q"] >> Q;

  cout << "Left camera matrix: " << K1 << endl;
  cout << "Right camera matrix: " << K2 << endl;
  cout << "Left distortion coefficients: " << D1 << endl;
  cout << "Right distortion coefficients: " << D2 << endl;
  cout << "Rotation matrix: " << R << endl;
  cout << "Translation vector: " << T << endl;
  cout << "Rectification matrix 1: " << R1 << endl;
  cout << "Rectification matrix 2: " << R2 << endl;
  cout << "Projection matrix 1: " << P1 << endl;
  cout << "Projection matrix 2: " << P2 << endl;
  cout << "Disparity-to-depth mapping matrix: " << Q << endl;


  StereoCamera camera(0, 1, 640, 480, 30, 10, 5);
  camera.start();
  cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);

  double fps = 0.0;
  int frameCount = 0;
  double tickFrequency = cv::getTickFrequency();
  int64 startTime = cv::getTickCount();
  TicTocTimer timer;
  while (true) {
    timer.tic();
    auto images = camera.getLatestFrames();
    float dt = timer.toc().ms().value<float>();
    img1 = *images.first;
    img2 = *images.second;

    if (img1.empty() || img2.empty()) {
      cout << "No frame captured" << endl;
      continue;
    }


    cv::Mat lmapx, lmapy, rmapx, rmapy;
    cv::Mat imgU1, imgU2;

    cv::initUndistortRectifyMap(K1, D1, R1, P1, img1.size(), CV_32F, lmapx, lmapy);
    cv::initUndistortRectifyMap(K2, D2, R2, P2, img2.size(), CV_32F, rmapx, rmapy);
    cv::remap(img1, imgU1, lmapx, lmapy, cv::INTER_LINEAR);
    cv::remap(img2, imgU2, rmapx, rmapy, cv::INTER_LINEAR);


    // cv::namedWindow("Left Undistorted Image", cv::WINDOW_NORMAL);
    // cv::imshow("Left Undistorted Image", imgU1);
    // cv::namedWindow("Right Undistorted Image", cv::WINDOW_NORMAL);
    // cv::imshow("Right Undistorted Image", imgU2);
    // cv::waitKey(0);
    // Create StereoBM object

    // convert to grayscale
    cv::Mat imgU1Gray;
    cv::Mat imgU2Gray;
    cv::cvtColor(imgU1, imgU1Gray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(imgU2, imgU2Gray, cv::COLOR_BGR2GRAY);

    // int numDisparities = 16 * 5; // must be divisible by 16
    int numDisparities = 16 * 5; // must be divisible by 16
    int blockSize = 17;          // must be odd and >= 5
    cv::Ptr<cv::StereoBM> stereo = cv::StereoBM::create(numDisparities, blockSize);
    stereo->setPreFilterCap(31);
    stereo->setTextureThreshold(6);
    stereo->setUniquenessRatio(6);
    stereo->setSpeckleWindowSize(100);
    stereo->setSpeckleRange(2);

    // use gaussian blur
    cv::GaussianBlur(imgU1Gray, imgU1Gray, cv::Size(9, 9), 0);
    cv::GaussianBlur(imgU2Gray, imgU2Gray, cv::Size(9, 9), 0);
    // Compute disparity map
    cv::Mat disparity16S, disparity8U;
    stereo->compute(imgU1Gray, imgU2Gray, disparity16S);

    // Normalize disparity map to 8-bit image for display
    disparity16S.convertTo(disparity8U, CV_8U, 255.0 / (numDisparities * 16.0));

    cv::Mat img;
    cv::hconcat(imgU1, imgU2, img);
    // cv::resize(img, img, cv::Size(320, 240));
    timer.tic();
    // cv::imshow("Camera", img);
    cv::imshow("Camera", disparity8U);
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
    // cout << std::flush;
    // cout << "\rFPS: " << fps << std::flush;
    cout << "FPS: " << fps << std::endl;
    // std::this_thread::yield();
  }

  cout << "Undistortion and rectification completed successfully." << endl;

  return 0;
}