
#include "camera/mono.hpp"

#include "tictoc/timer.hpp"

using TimestampedFrame = MonoCamera::TimestampedFrame;

MonoCamera::MonoCamera(int camera_id, int width, int height, int fps, int buffer_count)
  : cap_(camera_id, cv::CAP_V4L2),
  // : cap_(camera_id, cv::CAP_GSTREAMER),
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
  buffer_count_(buffer_count),
  wait_for_new_frame_(false)
{
  init();
  just_written_.store(false);
}

MonoCamera::~MonoCamera() {
  if (!stopped_.load()) {
    stop();
  }
}

void MonoCamera::init() {
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
  std::thread(&MonoCamera::readyCamera, this).detach();
  std::cout << "Camera " << id_ << " ready thread started" << std::endl;
}

void MonoCamera::start() {
  stopped_.store(false);
  running_.store(true);
  while (!cap_ready_.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // std::this_thread::yield();
  }
  capture_thread_ = std::thread(&MonoCamera::captureLoop, this);
  std::cout << "Camera " << id_ << " capture thread started" << std::endl;
}

void MonoCamera::stop() {
  running_.store(false);
  stopped_.store(true);
  if (capture_thread_.joinable())
  capture_thread_.join();
  cap_ready_.store(false);
  std::cout << "Camera " << id_ << " capture thread stopped" << std::endl;
}

bool MonoCamera::ready() const {
  return cap_ready_.load();
}

int MonoCamera::getBufferCount() const {
  return buffer_count_;
}

TimestampedFrame& MonoCamera::getLatestFrame() {
  while (!cap_ready_.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // std::this_thread::yield();
  }

  while (running_.load()) {
    while (!just_written_.load()) {
      // std::cout << "Camera " << id_ << " waiting for frame" << std::endl;
      std::this_thread::yield();
    }
    just_written_.store(false);
    int index = write_index_.load();
    buffers_[index].used = true;
    return buffers_[index];
  }
  // while (running_.load()) {
  //   int index = write_index_.load();
  //   while (true) {
  //     if (buffers_[index].used) {
  //       std::this_thread::sleep_for(std::chrono::milliseconds(1));
  //       // std::this_thread::yield();
  //       index = write_index_.load();
  //       // std::cout << "Camera " << id_ << " waiting for frame" << std::endl;
  //     } else {
  //       break;
  //     }
  //   }
  //   buffers_[index].used = true;
  //   return buffers_[index];
  // }
  // if (running_.load()) {
  //   int index = write_index_.load();
  //   if (!buffers_[index].used) {
  //     // std::cout << "returning buffer " << index << std::endl;
  //     // buffers_[index].used = true;
  //     // std::unique_lock<std::mutex> lock(mtx_);
  //     // wait_for_new_frame_ = false;
  //     return buffers_[index];
  //   } else {
  //     // std::cout << "Camera " << id_ << " waiting for frame" << std::endl;
  //     // std::cout << "wait_for_new_frame_: " << wait_for_new_frame_ << std::endl;
  //     {
  //       std::unique_lock<std::mutex> lock(mtx_);
  //       wait_for_new_frame_ = true;
  //     }
  //     std::unique_lock<std::mutex> lock(mtx_);
  //     cv_.wait(lock, [this] { return !wait_for_new_frame_; });
  //     // wait_for_new_frame_ = false;
  //     // lock.unlock();
  //     int index = write_index_.load();
  //     // buffers_[index].used = true;
  //     if (buffers_[index].used) {
  //       // std::cout << "ERROR CAM " << id_ << std::endl;
  //     }
  //     return buffers_[index];
  //   }
  // }
  return buffers_[0];
}

TimestampedFrame& MonoCamera::getLatestFrameMinusOne() {
  while (!cap_ready_.load()) {
    // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::this_thread::yield();
  }
  int index = (write_index_.load() - 1 + buffer_count_) % buffer_count_;
  // last_read_index_.store(index);
  return buffers_[index];
}

TimestampedFrame& MonoCamera::getLatestFrameMinusN(int n) {
  while (!cap_ready_.load()) {
    // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::this_thread::yield();
  }
  int index = (write_index_.load() - n + buffer_count_) % buffer_count_;
  // last_read_index_.store(index);
  return buffers_[index];
}

void MonoCamera::readyCamera() {
  while (!cap_ready_.load()) {
    cap_ >> buffers_[0].frame;
    if (!buffers_[0].frame.empty()) {
      cap_ready_.store(true);
      std::cout << "Camera " << id_ << " ready to capture frames" << std::endl;
      break;
    }
  }
}

void MonoCamera::captureLoop() {
  int next_index = 0;
  TicToc timer;
  while (running_.load()) {
    // int next_index = (write_index_.load() + 1) % buffer_count_;
    timer.tic();
    cap_ >> buffers_[next_index].frame;
    // std::cout << "Camera " << id_ << " capture loop time: " << timer.toc().ms().value<float>() << std::endl;

    if (!buffers_[next_index].frame.empty()) {
      buffers_[next_index].used = false;
      buffers_[next_index].timestamp = std::chrono::steady_clock::now();
      write_index_.store(next_index);
      just_written_.store(true);
      // timer.tic();
      // {
      //   std::lock_guard<std::mutex> lock(mtx_);
      //   wait_for_new_frame_ = false;
      // }
      // cv_.notify_one();
      // std::cout << "Notify one dt: " << timer.toc().ms().value<float>() << std::endl;
      // next_index = (write_index_.load() + 1) % buffer_count_;
      next_index = (next_index + 1) % buffer_count_;
    } else {
      std::cerr << "Camera " << id_ << " failed to capture frame" << std::endl;
    }
  }
}