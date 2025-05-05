#include <opencv2/opencv.hpp>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>

#include "tictoc/timer.hpp"

const char* SHM_NAME = "/opencv_image_shm";
const char* SEM_NAME = "/opencv_image_sem";

const int WIDTH = 640;
const int HEIGHT = 480;
const int CHANNELS = 3;
const size_t IMAGE_SIZE = WIDTH * HEIGHT * CHANNELS;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    void* shm_ptr = mmap(nullptr, IMAGE_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    sem_t* sem = sem_open(SEM_NAME, 0);

    if (sem == SEM_FAILED) {
      perror("sem_open failed");
      exit(1);
  }

  int sval = 0;
  while (true) {
      if (sem_getvalue(sem, &sval) == -1) {
          perror("sem_getvalue failed");
          break;
      }
      if (sval <= 0) break;
      sem_wait(sem);  // Safe: we know sval > 0
  }

    std::cout << "shm_fd: " << shm_fd << std::endl;
    cv::Mat frame(HEIGHT, WIDTH, CV_8UC3);

    TicToc timer;
    while (true) {
        timer.tic();
        sem_wait(sem);
        std::cout << "Display time: " << timer.toc().ms().value<float>() << " ms" << std::endl;
        std::memcpy(frame.data, shm_ptr, IMAGE_SIZE);

        cv::imshow("Shared Memory Image", frame);
        if (cv::waitKey(1) == 27) break; // ESC to quit
    }

    munmap(shm_ptr, IMAGE_SIZE);
    close(shm_fd);
    sem_close(sem);
    return 0;
}
