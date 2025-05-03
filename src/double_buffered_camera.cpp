#include "camera/double_buffered_camera.hpp"

using TimestampedFrame = DoubleBufferedCamera::TimestampedFrame;

DoubleBufferedCamera::DoubleBufferedCamera(int camera_id, int width, int height, int fps)
: cap_(camera_id, cv::CAP_V4L2),
write_index_(0),
running_(false),
cap_ready_(false),
id_(camera_id),
width_(width),
height_(height),
fps_(fps)
{
  init();
}

DoubleBufferedCamera::~DoubleBufferedCamera()
{
  stop();
}

void DoubleBufferedCamera::init() {

  count_ = 0;
  buffers_[0].frame = cv::Mat();
  buffers_[1].frame = cv::Mat();

  if (!cap_.isOpened()) {
    std::cerr << "Failed to open camera " << id_ << std::endl;
    return;
  }

  // Set camera properties
  cap_.set(cv::CAP_PROP_FRAME_WIDTH, width_);
  cap_.set(cv::CAP_PROP_FRAME_HEIGHT, height_);
  cap_.set(cv::CAP_PROP_FPS, fps_);

  double width = cap_.get(cv::CAP_PROP_FRAME_WIDTH);
  double height = cap_.get(cv::CAP_PROP_FRAME_HEIGHT);
  double fps = cap_.get(cv::CAP_PROP_FPS);
  if (width != width_ || height != height_ || fps != fps_) {
    std::cout << "Failed to set camera " << id_ << " properties" << std::endl;
  }
  std::cout << "Camera " << id_ << ": " << width << "x" << height << " @ " << fps << " FPS" << std::endl;

  std::thread ready_thread(&DoubleBufferedCamera::readyCamera, this);
  ready_thread.detach(); // detach the thread to run independently
  std::cout << "Camera " << id_ << " ready thread started" << std::endl;
}

void DoubleBufferedCamera::start() {
  running_ = true;
  while (!cap_ready_.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  capture_thread_ = std::thread(&DoubleBufferedCamera::captureLoop, this);
  std::cout << "Camera " << id_ << " capture thread started" << std::endl;
}

void DoubleBufferedCamera::stop() {
  running_ = false;
  if (capture_thread_.joinable())
  capture_thread_.join();
  cap_ready_ = false;
  std::cout << "Camera " << id_ << " capture thread stopped" << std::endl;
}

bool DoubleBufferedCamera::ready() const {
  return cap_ready_.load();
}

TimestampedFrame DoubleBufferedCamera::getLatestFrame() {
  while (!cap_ready_.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  int index = write_index_.load();
  return buffers_[index];
}

void DoubleBufferedCamera::readyCamera() {
  while (!cap_ready_) {
    cap_ >> buffers_[0].frame;
    if (!buffers_[0].frame.empty()) {
      cap_ready_.store(true);
      std::cout << "Camera " << id_ << " ready to capture frames" << std::endl;
      break;
    }
  }
}

void DoubleBufferedCamera::captureLoop() {

  while (running_) {
    int next_index = 1 - write_index_.load(); // write to inactive buffer
    cap_ >> buffers_[next_index].frame;
    if (buffers_[next_index].frame.empty()) {
      std::cerr << "Camera " << id_ << " failed to caputure frame" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }

    if (!buffers_[next_index].frame.empty()) {
      buffers_[next_index].timestamp = std::chrono::steady_clock::now();
      write_index_.store(next_index); // atomic flip
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}