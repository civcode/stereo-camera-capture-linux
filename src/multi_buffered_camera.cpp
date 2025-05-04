
#include "camera/multi_buffered_camera.hpp"

using TimestampedFrame = MultiBufferedCamera::TimestampedFrame;

MultiBufferedCamera::MultiBufferedCamera(int camera_id, int width, int height, int fps, int buffer_count)
  : cap_(camera_id, cv::CAP_V4L2),
  write_index_(0),
  running_(false),
  cap_ready_(false),
  stopped_(true),
  last_read_index_(-1),
  cap_count_(0),
  id_(camera_id),
  width_(width),
  height_(height),
  fps_(fps),
  buffer_count_(buffer_count)
{
  init();
}

MultiBufferedCamera::~MultiBufferedCamera() {
  if (!stopped_.load()) {
    stop();
  }
}

void MultiBufferedCamera::init() {
  count_ = 0;
  buffers_.resize(buffer_count_);

  for (auto& buf : buffers_) {
    buf.frame = cv::Mat();
  }

  if (!cap_.isOpened()) {
    std::cerr << "Failed to open camera " << id_ << std::endl;
    return;
  }

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
  std::thread(&MultiBufferedCamera::readyCamera, this).detach();
  std::cout << "Camera " << id_ << " ready thread started" << std::endl;
}

void MultiBufferedCamera::start() {
  stopped_.store(false);
  running_.store(true);
  while (!cap_ready_.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  capture_thread_ = std::thread(&MultiBufferedCamera::captureLoop, this);
  std::cout << "Camera " << id_ << " capture thread started" << std::endl;
}

void MultiBufferedCamera::stop() {
  running_.store(false);
  stopped_.store(true);
  if (capture_thread_.joinable())
  capture_thread_.join();
  cap_ready_.store(false);
  std::cout << "Camera " << id_ << " capture thread stopped" << std::endl;
}

bool MultiBufferedCamera::ready() const {
  return cap_ready_.load();
}

int MultiBufferedCamera::getBufferCount() const {
  return buffer_count_;
}

TimestampedFrame& MultiBufferedCamera::getLatestFrame() {
  while (!cap_ready_.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // static thread_local int last_read_index = -1;

  while (running_.load()) {
    int current_index = write_index_.load();
    if (current_index != last_read_index_.load()) {
      last_read_index_.store(current_index);
      return buffers_[current_index];
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return buffers_[0];
}

TimestampedFrame& MultiBufferedCamera::getLatestFrameMinusOne() {
  while (!cap_ready_.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  int index = (write_index_.load() - 1 + buffer_count_) % buffer_count_;
  // last_read_index_.store(index);
  return buffers_[index];
}

TimestampedFrame& MultiBufferedCamera::getLatestFrameMinusN(int n) {
  while (!cap_ready_.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  int index = (write_index_.load() - n + buffer_count_) % buffer_count_;
  // last_read_index_.store(index);
  return buffers_[index];
}

void MultiBufferedCamera::readyCamera() {
  while (!cap_ready_.load()) {
    cap_ >> buffers_[0].frame;
    if (!buffers_[0].frame.empty()) {
      cap_ready_.store(true);
      std::cout << "Camera " << id_ << " ready to capture frames" << std::endl;
      break;
    }
  }
}

void MultiBufferedCamera::captureLoop() {
  while (running_.load()) {
    int next_index = (write_index_.load() + 1) % buffer_count_;
    cap_ >> buffers_[next_index].frame;

    if (!buffers_[next_index].frame.empty()) {
      buffers_[next_index].timestamp = std::chrono::steady_clock::now();
      write_index_.store(next_index);
    } else {
      std::cerr << "Camera " << id_ << " failed to capture frame" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}