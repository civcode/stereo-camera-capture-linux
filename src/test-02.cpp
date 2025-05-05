#include <opencv2/opencv.hpp>
#include <thread>

#include <pthread.h>
#include <sched.h>

#include "tic_toc_timer.h"

void capture() {
  // // Set CPU affinity
  //   cpu_set_t cpuset;
  //   CPU_ZERO(&cpuset);
  //   CPU_SET(1, &cpuset);  // Pin to CPU 1
  //   pthread_t this_thread = pthread_self();
  //   if (pthread_setaffinity_np(this_thread, sizeof(cpu_set_t), &cpuset) != 0) {
  //       perror("pthread_setaffinity_np");
  //   }

  //   // Set real-time scheduling
  //   sched_param sch;
  //   sch.sched_priority = 20;  // Priority (1–99 for SCHED_FIFO/RR)
  //   if (pthread_setschedparam(this_thread, SCHED_RR, &sch) != 0) {
  //       perror("pthread_setschedparam");
  //   }

  cv::VideoCapture cap(0, cv::CAP_V4L2);
  if (!cap.isOpened()) {
    std::cerr << "Error: Could not open camera." << std::endl;
    return;
  }

  cv::Mat frame;
  TicTocTimer timer;
  while (true) {
    timer.tic();
    cap >> frame;
    std::cout << "Capture time: " << timer.toc().ms().value<float>() << " ms" << std::endl;
    if (frame.empty()) {
      std::cerr << "Error: No frame captured." << std::endl;
      continue;
    }
  }
}

int main(int argc, char* argv[]) {

  std::thread capture_thread(capture);

  while (true) {
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // std::this_thread::yield();
    cv::waitKey(1);

  };
  capture_thread.join();

  return 0;
}