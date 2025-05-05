#include <opencv2/opencv.hpp>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "tictoc/timer.hpp"

const char* SHM_NAME = "/opencv_image_shm";
const char* SEM_NAME = "/opencv_image_sem";

const int WIDTH = 640;
const int HEIGHT = 480;
const int CHANNELS = 3;
const size_t IMAGE_SIZE = WIDTH * HEIGHT * CHANNELS;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    int ret = ftruncate(shm_fd, IMAGE_SIZE);
    void* shm_ptr = mmap(nullptr, IMAGE_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    sem_t* sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);

    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Camera open failed\n";
        return 1;
    }

    cv::Mat frame;
    TicToc timer;
    while (true) {
        timer.tic();
        cap >> frame;
        std::cout << "Capture time: " << timer.toc().ms().value<float>() << " ms" << std::endl;
        if (frame.empty()) continue;

        cv::resize(frame, frame, cv::Size(WIDTH, HEIGHT));
        std::memcpy(shm_ptr, frame.data, IMAGE_SIZE);

        sem_post(sem); // Signal new frame available
        // cv::waitKey(30);
    }

    munmap(shm_ptr, IMAGE_SIZE);
    close(shm_fd);
    sem_close(sem);
    return 0;
}
