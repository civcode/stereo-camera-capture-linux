cmake_minimum_required(VERSION 3.10)
project(Camera)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)
# set compilter options
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -O3")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

include(FetchContent)
FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG        v3.0.0
)
FetchContent_Declare(
  tictoctimer
  GIT_REPOSITORY https://github.com/civcode/tic-toc-timer.git
  GIT_TAG        v0.2.2
)
FetchContent_MakeAvailable(cxxopts)
FetchContent_MakeAvailable(tictoctimer)

find_package(OpenCV REQUIRED)

include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${OpenCV_INCLUDE_DIRS}
)

set(SOURCES
  ${CMAKE_SOURCE_DIR}/src/double_buffered_camera.cpp
  ${CMAKE_SOURCE_DIR}/src/multi_buffered_camera.cpp
  ${CMAKE_SOURCE_DIR}/src/stereo_camera.cpp
)

add_executable(camera src/main.cpp ${SOURCES})
target_link_libraries(camera ${OpenCV_LIBS} cxxopts::cxxopts tictoctimer)

add_executable(test-01 src/test-01.cpp ${SOURCES})
target_link_libraries(test-01 ${OpenCV_LIBS} cxxopts::cxxopts tictoctimer)

add_executable(test-02 src/test-02.cpp ${SOURCES})
target_link_libraries(test-02 ${OpenCV_LIBS} cxxopts::cxxopts tictoctimer)

add_executable(test-03 src/test-03.cpp ${SOURCES})
target_link_libraries(test-03 ${OpenCV_LIBS} cxxopts::cxxopts tictoctimer)

add_executable(ipc_capture src/ipc_capture.cpp ${SOURCES})
target_link_libraries(ipc_capture ${OpenCV_LIBS} cxxopts::cxxopts tictoctimer)

add_executable(ipc_visualize src/ipc_visualize.cpp ${SOURCES})
target_link_libraries(ipc_visualize ${OpenCV_LIBS} cxxopts::cxxopts tictoctimer)

add_executable(rectify src/rectify.cpp ${SOURCES})
target_link_libraries(rectify ${OpenCV_LIBS} cxxopts::cxxopts tictoctimer)